setlocal
if not exist "build\" mkdir build
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat
cd build
call cl -FC -Zi ..\ray.cc  user32.lib gdi32.lib 
endlocal
