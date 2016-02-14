call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

REM cl %1

REM /subsystem:windows,5.1 (32-bit), -EHa-
set FLAGS=-Oi -EHsc -WX -W4 -wd4201 -wd4189 -wd4100 -wd4127 -wd4189 -wd4505 -wd4996 /FC /Z7 /nologo
set LINKER_FLAGS=/opt:ref /subsystem:windows
set LIBS=glew32s.lib opengl32.lib glu32.lib gdi32.lib kernel32.lib user32.lib

IF NOT EXIST build mkdir build
pushd build
REM cl -DAAENGINE_DEBUG=1 -DAAMATH_DEBUG=1 -DAAENGINE_INTERNAL=1 -Oi -EHa- -WX -W4 -wd4201 -wd4100 -wd4189 /FC /Z7 /Tp..\src\win32_platform.cpp /link -subsystem:windows glew32s.lib opengl32.lib glu32.lib gdi32.lib kernel32.lib user32.lib
cl -DAAENGINE_DEBUG=1 -DAAMATH_DEBUG=1 -DAAENGINE_INTERNAL=1 %FLAGS% ..\src\win32_platform.cpp /link %LINKER_FLAGS% %LIBS%
popd

REM -EHa- 
REM -WX -W4 -wd4201 -wd4100 -wd4189
