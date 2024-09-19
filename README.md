# VulkanEngine
 A 3D Graphics/Game Engine

***Important Steps***
 1. In order to run this program, you need to install and configure VulkanSDK/GLFW/GLM. You can visit the Vulkan Tutorial website to go through the steps: https://vulkan-tutorial.com/Development_environment | You will also need to know how to compile the .vert and .frag files ***This is Important*** : You may have to go to the Shaders folder in the main project folder and run the compile.bat file (just double click on it). As of now, the files are compiled and the .spv files are already complete, so it should be fine, however, if you change anything in those files, you will need to recompile them. Also, if you are trouble shooting, keep in mind that those files do not compile on their own or through your IDE. The compile.bat file will clean and compile the .vert and .frag files as your IDE/system cannot compile them. The reason for this is that it is not written in C++ but in GLSL which is a graphics card language similar to C++. Also, if you edit them, I recommend that you do so in notepad as Visual Studio or other IDE's might insert BOM into the file rather than UTF-8 encoding. If you are getting any compiler errors after running the compile.bat file, it is because of that. To fix this, open the .vert/.frag file in question in notepad, make sure all the code is present and correct, and then save it. This will correct the encoding.
 
 2. You must be using C++17, there are a few expressions that are not available in earlier versions. You could adjust them for your version if you prefer. 
 
 3. This program uses tiny object loader, which is header file that contains code for loading in obj files. You will need to get this file and include it in the project. Here's a URL for the file: https://github.com/tinyobjloader/tinyobjloader | you only need the tiny_obj_loader.h file in the list. If you're having trouble including it in the project, just add it to the actual project folder and reference it that way, you may have to change the <> with "" for the include header which is located in the Model.cpp file. 
 
 4. The actual obj files that are in this project can be found here: https://drive.google.com/file/d/1ysiOzOamHn2rr_CIRmAX4QgDUl7jNNy5/view?usp=drive_link | just unzip the folder and put it in the main project folder. If you need to update the file path in the program, you can do that in the Application.cpp file in the loadGameObjects function. These file paths are for each of the models, you can change them to match where you put the folder. Note that you may have to include a full path if it is not in the main project folder. Also, the program loads up 2 models, the car and the quad(plane). The other ones are commented out, if you wish to load them in, simply uncomment them. You can also add more game objects if you wish by copying the code for a model and pasting it then changing the file path and the variable names.

 Controls (when you get the program running a window with a couple 3D models will appear):

 For keyborad: WASDQE (forward, backward, left, right, up, down)

 For Mouse (works best imo):
 Move: Hold middle mouse button + mouse motion
 Rotate Camera: Hold right mouse button + mouse motion
 Zoom: Scroll wheel
                        ***When trying to rotate around an object, it's easier to hold the***
                        ***middle mouse button and the left mouse button at the same time***
                        ***This will give you control over rotation and translation at once***
 
 ***More Info***
 In the actual code, I did not go into detail explaining the validation layers because honestly I don't understand them. The purpose of them is to help with the build process so that any mistakes that you make will be caught so that you're not left in the dark. Although I know how to use it, any in depth knowledge of how it works can be found in the URL provided. Most of the validation layers can be found in the Device.cpp file.

 This file is structured in a classic C++ way. There are .h files and .cpp files. Some of the function definitions are in the header files if they are small enough and most will be in the corresponding cpp file.

 If you wish to change the present mode, or rather check for additional present modes that are supported by your graphics cards, go to the SwapChain.cpp file and see the chooseSwapPresentMode function. There, you will see that the program checks for mailbox present mode and then defaults to FIFO(V-sync). If you wish to check for Immediate present mode, simply uncomment the chunk of code from line 404 to 409. After implementing the delta time feature, the objects' movement should remain consistent regardless of the present mode.

 If you have an AMD graphics card, you may get some validation warnings in the console. These are just the validation layers looking for a render pass that hasn't been created yet. The render pass does get created and that warning goes away. Also this is using a more recent version of Vulkan (version 1.1) and so I'm not too sure on the changes to the validation layers. Most of the Vulkan documentation and general knowledge revolves around version 1.0. This includes the validation layers. From what I've been able to gather, Vulkan is  buggy with AMD. If you have an NVIDIA graphics card, please let me know how it runs.

To have the best chance of understanding the program, go to the Application.h/Application.cpp files. This is the heart of the program and is what all of the builder classes connect to. From there, you will find extensive commentary to help you understand what everything does. The entire program loop runs in the run() function in the Application.h/Application.cpp file. If you start at the top of the h file, you will see some objects being created which in turn start an extensive build process. This URL is a great visualization of the graphics pipeline steps and how they connect: https://vulkan.lunarg.com/doc/view/latest/windows/tutorial/html/14-init_pipeline.html | Note that the GameObject class uses the Model class to create models and so the Application class will often use the Model class through the GameObject class.

Follow the functions in the Application class to get a good understanding of the flow of the program. I did my best to include as much commentary as possible in order to explain how it all works, but in the end, a graphics engine (especially Vulkan) is very complicated and requires a lot of study. If you are in doubt, see the Vulkan API here: https://docs.vulkan.org/guide/latest/protected.html | Enjoy!

***Making OBJ files to render***
This program only takes wavefront obj files, load your model into blender and export using these settings:

![alt text](https://drive.google.com/file/d/11nbk_LXf-6smAOH3DBamFLvjYyUSrNkd/view?usp=sharing/Screenshot.png)