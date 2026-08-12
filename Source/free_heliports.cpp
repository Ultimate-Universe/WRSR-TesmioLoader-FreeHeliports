// FreeHeliports - TesmioLoader plugin
// v1.1.0, target SOVIET64.exe v1.1.1.9, TesmioLoader API 4
//
// FreeHeliportParking keeps one genuine zero-cost construction-stage record.
// WRSR requires that metadata when validating helicopters of 10 tonnes or
// more, but a zero-work stage never advances by itself. This plugin intercepts
// the native construction updater, recognises only FreeHeliportParking, marks
// the new instance complete, and then forwards it through WRSR's own completion
// path. Vehicle compatibility, helicopter ownership and HDO are untouched.

#define TSM_API_VERSION 4u
#define EXPORT extern "C" __declspec(dllexport)

typedef __SIZE_TYPE__ usize;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

extern "C" int _fltused = 0;

typedef int BOOL;
typedef unsigned long DWORD;
typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* LPVOID;

extern "C" __declspec(dllimport) BOOL __stdcall VirtualProtect(
    LPVOID address, usize size, DWORD newProtect, DWORD* oldProtect);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(
    HANDLE process, const void* address, usize size);
extern "C" __declspec(dllimport) HANDLE __stdcall GetCurrentProcess(void);

extern "C" BOOL __stdcall DllMain(HINSTANCE, DWORD, LPVOID) { return 1; }
extern "C" void* volatile g_freeHeliportsRelocationAnchor = (void*)&DllMain;

struct TsmHost
{
    unsigned apiVersion;
    unsigned structSize;
    void* exeModule;
    u8* exeBase;
    usize exeSize;
    void* engineModule;
    const char* baseDir;
    const char* pluginDir;
    void (*log)(const char* fmt, ...);
    void** (*findIatSlot)(void* module, const char* dll, const char* fn);
    int (*patchIat)(void* module, const char* dll, const char* fn,
                    void* detour, void** original, const char* label);
    int (*installInlineHook)(void* target, void* detour, void** trampoline,
                             const u8* expect, usize stolen,
                             const char* label);
    u8* (*allocNear)(u8* anchor, usize size);
    int (*readablePtr)(const void* p, usize n);
    long (*faultFilter)(const char* what, void* exceptionPointers);
    int (*configInt)(const char* iniName, const char* section,
                     const char* key, int fallback);
    int (*configString)(const char* iniName, const char* section,
                        const char* key, char* out, int outSize,
                        const char* fallback);
    int (*provide)(const char* service, unsigned version, const void* iface);
    const void* (*consume)(const char* service, unsigned version);
    const char* vfsRoot;
};

struct TsmPluginInfo
{
    const char* name;
    const char* version;
};

static const TsmHost* H = 0;
static u8* EXE = 0;

// Audited SOVIET64.exe v1.1.1.9 construction updater and structure offsets.
static const usize RVA_CONSTRUCTION_UPDATE = 0x00159070u; // FUN_140159070

static const usize B_TYPEDESC       = 0x0318u;
static const usize B_PROGRESS       = 0x0604u;
static const usize B_STAGE_BEGIN    = 0x0610u;
static const usize B_STAGE_END      = 0x0618u;
static const usize B_COMPLETING     = 0x0EA8u;
static const usize TD_TYPE          = 0x0360u;
static const usize TD_STAGE_BEGIN   = 0x0370u;
static const usize TD_STAGE_END     = 0x0378u;
static const usize STAGE_RECORD_SIZE = 0x21B8u;

static const int TYPE_AIRPLANE_PARKING = 0x2F;
static const char TARGET_ASSET[] = "FreeHeliportParking";

typedef void (*FnConstructionUpdate)(void* game, void* building,
                                     float progressPulse, char mode);
static FnConstructionUpdate o_ConstructionUpdate = 0;

static int IsCanonicalUserRange(const void* p, usize n)
{
    const u64 first = (u64)p;
    const u64 userMax = 0x00007FFFFFFFFFFFull;
    if (first < 0x10000ull || first > userMax) return 0;
    if (n == 0) return 1;
    const u64 span = (u64)n - 1ull;
    return span <= userMax - first;
}

static int IsReadable(const void* p, usize n)
{
    return H && H->readablePtr && IsCanonicalUserRange(p, n) &&
           H->readablePtr(p, n);
}

static void* ReadPointer(void* base, usize offset)
{
    u8* p = (u8*)base;
    if (!p || !IsReadable(p + offset, sizeof(void*))) return 0;
    return *(void**)(p + offset);
}

static int ReadInt(void* base, usize offset, int* out)
{
    u8* p = (u8*)base;
    if (!p || !out || !IsReadable(p + offset, sizeof(int))) return 0;
    *out = *(int*)(p + offset);
    return 1;
}

static int HasValidStageVector(void* base, usize beginOffset, usize endOffset)
{
    u8* object = (u8*)base;
    if (!object || !IsReadable(object + beginOffset, 16)) return 0;

    u8* begin = *(u8**)(object + beginOffset);
    u8* end = *(u8**)(object + endOffset);
    if (!begin || !end || end <= begin) return 0;

    const usize bytes = (usize)(end - begin);
    if ((bytes % STAGE_RECORD_SIZE) != 0) return 0;
    const usize count = bytes / STAGE_RECORD_SIZE;
    if (count == 0 || count > 32) return 0;
    return IsReadable(begin, bytes);
}

static int IsNameBoundary(u8 value)
{
    return value == 0 || value == '/' || value == '\\';
}

static int DescriptorNamesTarget(void* descriptor)
{
    if (!descriptor) return 0;
    const u8* text = (const u8*)descriptor;

    usize needleLength = 0;
    while (TARGET_ASSET[needleLength]) ++needleLength;

    // Workshop descriptors may prefix the internal name with an item path.
    for (usize i = 0; i + needleLength <= 494; ++i)
    {
        if (!IsReadable(text + i, needleLength + 1)) return 0;
        if (i != 0 && !IsNameBoundary(text[i - 1])) continue;

        usize j = 0;
        while (j < needleLength && text[i + j] == (u8)TARGET_ASSET[j]) ++j;
        if (j == needleLength && IsNameBoundary(text[i + needleLength])) return 1;
    }
    return 0;
}

static int ShouldCompleteImmediately(void* building)
{
    u8* b = (u8*)building;
    if (!b || !IsReadable(b, B_COMPLETING + 1)) return 0;
    if (b[B_COMPLETING] != 0) return 0;

    const float progress = *(float*)(b + B_PROGRESS);
    if (!(progress >= 0.0f && progress < 1.0f)) return 0;

    void* descriptor = ReadPointer(building, B_TYPEDESC);
    int type = -1;
    if (!descriptor || !ReadInt(descriptor, TD_TYPE, &type) ||
        type != TYPE_AIRPLANE_PARKING || !DescriptorNamesTarget(descriptor))
        return 0;

    // Both static type metadata and the new live construction instance must
    // contain a real stage. This prevents the plugin from manufacturing an
    // invalid stage-less building state.
    return HasValidStageVector(descriptor, TD_STAGE_BEGIN, TD_STAGE_END) &&
           HasValidStageVector(building, B_STAGE_BEGIN, B_STAGE_END);
}

static void h_ConstructionUpdate(void* game, void* building,
                                 float progressPulse, char mode)
{
    if (ShouldCompleteImmediately(building))
    {
        *(float*)((u8*)building + B_PROGRESS) = 1.0f;
        progressPulse = 0.1f;
        if (H && H->log)
            H->log("freeheli  native instant completion triggered for FreeHeliportParking");
    }

    o_ConstructionUpdate(game, building, progressPulse, mode);
}

static int InstallVanillaHook(u8* target)
{
    if (!target || !H || !H->installInlineHook || !IsReadable(target, 16)) return 0;

    // Exact WRSR 1.1.1.9 prologue at SOVIET64.exe + 0x159070.
    // Instruction lengths are 3 + 4 + 5 + 4 = 16 bytes, so the inline-hook
    // trampoline never splits an instruction. Refuse unknown game builds.
    static const u8 expected[16] =
    {
        0x48, 0x8B, 0xC4,
        0x44, 0x88, 0x48, 0x20,
        0xF3, 0x0F, 0x11, 0x50, 0x18,
        0x48, 0x89, 0x50, 0x10,
    };

    for (usize i = 0; i < sizeof(expected); ++i)
        if (target[i] != expected[i])
        {
            if (H->log)
                H->log("freeheli  construction hook preflight refused: WRSR 1.1.1.9 prologue mismatch");
            return 0;
        }

    return H->installInlineHook(target, (void*)h_ConstructionUpdate,
                                (void**)&o_ConstructionUpdate,
                                expected, sizeof(expected),
                                "FreeHeliports construction completion");
}

static int IsAbsoluteIndirectJump(const u8* target)
{
    return target && IsReadable(target, 14) &&
           target[0] == 0xFF && target[1] == 0x25 &&
           target[2] == 0 && target[3] == 0 &&
           target[4] == 0 && target[5] == 0;
}

static int ChainExistingHook(u8* target)
{
    if (!IsAbsoluteIndirectJump(target)) return 0;
    void** slot = (void**)(target + 6);
    void* existing = *slot;
    if (!existing || existing == (void*)h_ConstructionUpdate ||
        !IsReadable(existing, 16))
        return 0;

    DWORD oldProtect = 0;
    if (!VirtualProtect(slot, sizeof(void*), 0x40u, &oldProtect)) return 0;
    o_ConstructionUpdate = (FnConstructionUpdate)existing;
    *slot = (void*)h_ConstructionUpdate;
    DWORD ignored = 0;
    VirtualProtect(slot, sizeof(void*), oldProtect, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));

    if (H && H->log)
        H->log("hook ok      FreeHeliports completion target=%p chained=%p",
               target, existing);
    return 1;
}

EXPORT unsigned TsmPluginApiVersion(void)
{
    return TSM_API_VERSION;
}

EXPORT int TsmPluginInit(const TsmHost* host, TsmPluginInfo* info)
{
    H = host;
    if (info)
    {
        info->name = "FreeHeliports";
        info->version = "1.1.0";
    }

    if (!H || H->apiVersion != TSM_API_VERSION || !H->exeBase ||
        !H->installInlineHook || !H->readablePtr)
        return 1;

    EXE = H->exeBase;
    u8* target = EXE + RVA_CONSTRUCTION_UPDATE;

    // TesmioLoader inline hooks use a standard FF 25 [rip+0] + qword target
    // stub. If another compatible plugin already owns this seam, wrap its
    // callable target. Unknown modifications are left untouched.
    int installed = IsAbsoluteIndirectJump(target)
                  ? ChainExistingHook(target)
                  : InstallVanillaHook(target);
    if (!installed)
    {
        if (H->log) H->log("freeheli  FAILED to install construction completion hook");
        return 1;
    }

    if (H->log)
        H->log("freeheli  v1.1.0 initialized for WRSR 1.1.1.9: native construction completion path active; vehicle compatibility untouched");
    return 0;
}
