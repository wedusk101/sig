@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

REM q:quiet m:minimum n:normal
set "PARAMS=/m /nr:false /nologo /verbosity:q /t:Build"

rem ========== build cmds ==========
call "cleanall.bat"
call :build siglibs.sln Release x86
call :build siglibs.sln ReleaseDLL x86
call :build siglibs.sln Debug x86
call :build siglibs.sln Release x64
call :build siglibs.sln ReleaseDLL x64
call :build siglibs.sln Debug x64
call "clean.bat"
pause
goto end

rem ========== functions ==========
:build
SETLOCAL
@echo Building: %1 %2 %3...
MSBuild %1 %PARAMS% /p:Configuration=%2 /p:Platform=%3
ENDLOCAL
goto end

:end
