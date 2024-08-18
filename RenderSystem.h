#pragma once

#define WINDOW_WIDTH 3200					//Change these to alter the default window size
#define WINDOW_HEIGHT 1800

#include "Camera.h"
#include "Device.h"
#include "GameObject.h"
#include "Pipeline.h"
#include "FrameInfo.h"

// std
#include <memory>

namespace engine {

	class RenderSystem {
	private:
		Device& device;

		//Here we create a Pipeline fron Pipeline.h and pass in the compiled shader files that were compiled by the 
		//compile.bat and Vulkan. This is how we get the files from the graphics card and use them in our program
		//Pipeline also has a default configuration that we pass our values into in case there are no other values.
		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

	public:

		RenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RenderSystem();

		void renderGameObjects(
			FrameInfo& frameInfo, std::vector<GameObject> &gameObjects);

		RenderSystem(const RenderSystem&) = delete;				//Delete copy constructors
		RenderSystem& operator=(const RenderSystem&) = delete;
	};
}