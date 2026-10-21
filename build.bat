@echo off
rem
rem
cd /d "%~dp0"
rc /nologo /fo hxcpp_jit_setup.res hxcpp_jit_setup.rc
if errorlevel 1 exit /b 1
cl /nologo /O2 /MT /EHsc /std:c++17 /DUNICODE /D_UNICODE hxcpp_jit_setup.cpp hxcpp_jit_setup.res /Fe:hxcpp_JIT_setup.exe /link /SUBSYSTEM:WINDOWS urlmon.lib shell32.lib user32.lib advapi32.lib ole32.lib comctl32.lib uuid.lib gdi32.lib
if errorlevel 1 exit /b 1
echo Done: hxcpp_JIT_setup.exe