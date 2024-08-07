@echo off
echo Cleaning up old compiled shaders...
del /Q /F Shaders\*.spv

echo Compiling GLSL shaders...

rem Compile vertex shader
D:\C++Libraries\VulkanSDK\Bin\glslc.exe Shaders\SimpleShader.vert -o Shaders\SimpleShader.vert.spv

rem Compile fragment shader
D:\C++Libraries\VulkanSDK\Bin\glslc.exe Shaders\SimpleShader.frag -o Shaders\SimpleShader.frag.spv

pause