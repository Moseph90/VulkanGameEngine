//********************************************************************
// This class creates and instance of Window.h and calls it's funcitons
// This is the actual application that runs the program. This is where
// the entire program stems from and if you follow all of the object
// creations as well as the function calls, you can piece together how
// the entire program works. The order of objects created here in the
// header file are also very important to the functionality. They are
// created even before the constructor body is executed.
//********************************************************************

#pragma once

#define WINDOW_WIDTH 3200					//Change these to alter the default window size
#define WINDOW_HEIGHT 1800

#include "Device.h"
#include "GameObject.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "Window.h"

// std
#include <memory>

namespace engine {

	class Application {
	private:
		Window window{ WIDTH, HEIGHT, "Cobra Engine" };		//Lets create an instance of the Window class with the proper parameters
															//This will also create a window for us as that handled in the Window class
		Device device{ window };
		std::unique_ptr<SwapChain> swapChain;

		//Here we create a Pipeline fron Pipeline.h and pass in the compiled shader files that were compiled by the 
		//compile.bat and Vulkan. This is how we get the files from the graphics card and use them in our program
		//Pipeline also has a default configuration that we pass our values into in case there are no other values.
		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<GameObject> gameObjects;

		void loadGameObjects();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffers(int imageIndex);
		void renderGameObjects(VkCommandBuffer commandBuffer);

	public:
		static constexpr int WIDTH{ WINDOW_WIDTH };			//We do this so that these variables are accessable to other parts of the program
		static constexpr int HEIGHT{ WINDOW_HEIGHT };
		
		Application();
		~Application();

		Application(const Application&) = delete;				//Delete copy constructors
		Application& operator=(const Application&) = delete;

		void run();
	};
}