@echo off
@echo Input Shader File Name with type:
set /p input= 
ShaderCompiler.exe ../Resources/Shaders/%input%
pause