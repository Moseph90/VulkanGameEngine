@echo off
echo Cleaning up old compiled shaders...
del /Q /F *.spv

echo Compiling GLSL shaders...

rem Compile vertex shader
D:\C++Libraries\VulkanSDK\Bin\glslc.exe SimpleShader.vert -o SimpleShader.vert.spv
D:\C++Libraries\VulkanSDK\Bin\glslc.exe PointLight.vert -o PointLight.vert.spv

rem Compile fragment shader
D:\C++Libraries\VulkanSDK\Bin\glslc.exe SimpleShader.frag -o SimpleShader.frag.spv
D:\C++Libraries\VulkanSDK\Bin\glslc.exe PointLight.frag -o PointLight.frag.spv

pause