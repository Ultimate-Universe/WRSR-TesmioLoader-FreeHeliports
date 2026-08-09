// Minimal conventional PE import descriptor for the three Kernel32 functions
// used while chaining an already-installed TesmioLoader inline hook.
        .section .idata$2,"dr"
        .p2align 2
        .globl __IMPORT_DESCRIPTOR_KERNEL32
__IMPORT_DESCRIPTOR_KERNEL32:
        .long .Lilt@IMGREL
        .long 0
        .long 0
        .long .Ldllname@IMGREL
        .long .Liat@IMGREL

        .section .idata$3,"dr"
        .p2align 2
        .zero 20

        .section .idata$4,"dr"
        .p2align 3
.Lilt:
        .long .LhintVirtualProtect@IMGREL
        .long 0
        .long .LhintFlushInstructionCache@IMGREL
        .long 0
        .long .LhintGetCurrentProcess@IMGREL
        .long 0
        .quad 0

        .section .idata$5,"drw"
        .p2align 3
        .globl __imp_VirtualProtect
        .globl __imp_FlushInstructionCache
        .globl __imp_GetCurrentProcess
.Liat:
__imp_VirtualProtect:
        .long .LhintVirtualProtect@IMGREL
        .long 0
__imp_FlushInstructionCache:
        .long .LhintFlushInstructionCache@IMGREL
        .long 0
__imp_GetCurrentProcess:
        .long .LhintGetCurrentProcess@IMGREL
        .long 0
        .quad 0

        .section .idata$6,"dr"
        .p2align 1
.LhintVirtualProtect:
        .short 0
        .asciz "VirtualProtect"
.LhintFlushInstructionCache:
        .short 0
        .asciz "FlushInstructionCache"
.LhintGetCurrentProcess:
        .short 0
        .asciz "GetCurrentProcess"
.Ldllname:
        .asciz "KERNEL32.dll"
