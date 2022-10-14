@echo off

set CommonCompilerFlags=-diagnostics:column -WL -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -FC -Z7 -GS- -Gs9999999
set InternalCompilerFlags=-DKENGINE_INTERNAL=1 -DKENGINE_SLOW=1
set IncludeDirectories=-I..\lib\kengine\code

REM unreferenced formal parameter
set CommonCompilerFlags=%CommonCompilerFlags% -wd4100
REM unreferenced local variable
set CommonCompilerFlags=%CommonCompilerFlags% -wd4101
REM named type definition in parentheses
set CommonCompilerFlags=%CommonCompilerFlags% -wd4115
REM unnamed type definition in parentheses
set CommonCompilerFlags=%CommonCompilerFlags% -wd4116
REM local variable is initialized but not referenced
set CommonCompilerFlags=%CommonCompilerFlags% -wd4189
REM nonstandard extension used: nameless struct/union
set CommonCompilerFlags=%CommonCompilerFlags% -wd4201
REM possible loss of data
set CommonCompilerFlags=%CommonCompilerFlags% -wd4244
REM 'type cast': pointer truncation
set CommonCompilerFlags=%CommonCompilerFlags% -wd4311
REM 'type cast': conversion
set CommonCompilerFlags=%CommonCompilerFlags% -wd4312

set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref

IF NOT EXIST bin mkdir bin
pushd bin

if "%~1"=="build_release" (

	cl %CommonCompilerFlags% %IncludeDirectories% -O2 ..\src\krest.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags% Uuid.lib

) else (
	if "%~1"=="build_dependencies" (

		REM Preprocessor
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od ..\lib\kengine\code\win32_kengine_preprocessor.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags%
		pushd ..\lib\kengine\code
		..\..\..\bin\win32_kengine_preprocessor.exe kengine_types.h > kengine_generated.h
		..\..\..\bin\win32_kengine_preprocessor.exe kengine_debug.h > kengine_debug_generated.h
		..\..\..\bin\win32_kengine_preprocessor.exe win32_kengine_types.h > win32_kengine_generated.c
		popd
		pushd ..\src
		..\bin\win32_kengine_preprocessor.exe krest.h > generated.c
		popd

		REM Unit tests
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od ..\lib\kengine\code\win32_kengine_tests.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags%
		win32_kengine_tests.exe

		REM Win32 platform
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od ..\lib\kengine\code\win32_kengine_http.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags%

		REM stress test
		cl %CommonCompilerFlags% %InternalCompilerFlags% %IncludeDirectories% -MTd -Od ..\src\stress_test.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags%

	) else (

		cl %CommonCompilerFlags% %InternalCompilerFlags% %IncludeDirectories% -MTd -Od ..\src\krest.c -LD /link /NODEFAULTLIB %CommonLinkerFlags%
	)
)

popd