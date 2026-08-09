@echo off
setlocal

clang-cl /nologo /c /O2 /GS- /GR- /EHs-c- /Zl /Fo:free_heliports.obj free_heliports.cpp || exit /b 1
clang-cl /nologo /c /O2 /GS- /GR- /EHs-c- /Zl /Fo:tinycrt.obj tinycrt.cpp || exit /b 1
clang --target=x86_64-pc-windows-msvc -c kernel32_import.s -o kernel32_import.obj || exit /b 1
lld-link /nologo /dll /nodefaultlib /subsystem:windows /entry:DllMain /dynamicbase /highentropyva /nxcompat /out:free_heliports.dll free_heliports.obj tinycrt.obj kernel32_import.obj /include:__IMPORT_DESCRIPTOR_KERNEL32 || exit /b 1
python finalize_pe.py free_heliports.dll || exit /b 1
python verify_release.py free_heliports.dll || exit /b 1

echo Built free_heliports.dll v1.0.0
endlocal
