cd /d Executable
for /F "delims=" %%i in ('dir /b') do (rmdir "%%i" /s/q || del "%%i" /s/q)

call premake\premake5.exe vs2019
PAUSE