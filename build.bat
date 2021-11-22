@echo off
cd %~dp0

rem If "debug" is not supplied as the first argument to this script, we will build in release mode.

set CL_flags=-nologo -TC -Fo./bin/obj/
if "%1"=="debug" (
	set CL_flags=%CL_flags% -Zi -MTd -DDEBUG -Od
) else (
	set CL_flags=%CL_flags% -MT -O2
)

set LINK_flags=/OUT:./bin/MarieAssembler.exe /SUBSYSTEM:CONSOLE /nologo
if "%1"=="debug" (
	set LINK_flags=%LINK_flags% /DEBUG
) else (
	set LINK_flags=%LINK_flags%
)

if not exist bin mkdir bin
if not exist bin\obj mkdir bin\obj

@echo on
cl %CL_flags% ./src/win32_MarieAssembler.c ./src/MarieAssembler.c -link %LINK_flags%
@echo off
