# Building the plugin

The release DLL is a native Windows x64 TesmioLoader plugin built with LLVM and no C runtime dependency.

## Requirements

- `clang-cl`
- `clang`
- `lld-link`
- Python 3

## Build

From the `Source` directory on Windows, run:

```text
build_release.bat
```

Equivalent commands:

```text
clang-cl /nologo /c /O2 /GS- /GR- /EHs-c- /Zl /Fo:free_heliports.obj free_heliports.cpp
clang-cl /nologo /c /O2 /GS- /GR- /EHs-c- /Zl /Fo:tinycrt.obj tinycrt.cpp
clang --target=x86_64-pc-windows-msvc -c kernel32_import.s -o kernel32_import.obj
lld-link /nologo /dll /nodefaultlib /subsystem:windows /entry:DllMain /dynamicbase /highentropyva /nxcompat /out:free_heliports.dll free_heliports.obj tinycrt.obj kernel32_import.obj /include:__IMPORT_DESCRIPTOR_KERNEL32
python finalize_pe.py free_heliports.dll
python verify_release.py free_heliports.dll
```

`finalize_pe.py` adds standard Windows `VERSIONINFO` metadata for v1.1.0 and writes the PE checksum. `verify_release.py` checks the architecture, TesmioLoader exports, hardening flags, native directories, section permissions and release markers. The explicit two-byte alignment directives in `kernel32_import.s` are required for every `IMAGE_IMPORT_BY_NAME` record.

Copy the completed DLL to `ModFiles/plugins/free_heliports.dll` for release.
