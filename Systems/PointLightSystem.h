#pragma once

#include "../Camera.h"
#include "../Device.h"
#include "../GameObject.h"
#include "../Pipeline.h"
#include "../FrameInfo.h"

// std
#include <memory>

namespace engine {

	class PointLightSystem {
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

		PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		void render(FrameInfo& frameInfo);

		PointLightSystem(const PointLightSystem&) = delete;				//Delete copy constructors
		PointLightSystem& operator=(const PointLightSystem&) = delete;
	};
}