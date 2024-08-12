#include "RenderSystem.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace engine {
	struct SimplePushConstantData {
		glm::mat4 transform{ 1.0f };
		glm::mat4 normalMatrix{ 1.0f };
	};

	RenderSystem::RenderSystem(Device& tempDevice, VkRenderPass renderPass) : device{ tempDevice } {
		createPipelineLayout();		// This creates the layout and initializes the device object settings
		createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void RenderSystem::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;

		//Push constants are a way to send very small amounts of data to our shader program
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		//This is where the device is created and the layout is checked
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr,
			&pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void RenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		//Here we use the swapchain's width and height as it may not match the window's
		Pipeline::defultPipelineConfigInfo(pipelineConfig);

		//Render pass describes structure and format of our frame buffer objects and their
		//attachments. For example it tells the graphics pipeline what we will be using and
		//what to expect in our output frame buffers and how they work. This way it's
		//prepared to output to a frame buffer with the correct settings
		pipelineConfig.renderPass = renderPass;

		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(
			"Shaders/SimpleShader.vert.spv",		//These are the files written in GLSL for the graphics
			"Shaders/SimpleShader.frag.spv",		//and then comipled using the compile.bat file
			device,
			pipelineConfig);
	}
	void RenderSystem::renderGameObjects(
		FrameInfo& frameInfo, std::vector<GameObject> &gameObjects) {
		pipeline->bind(frameInfo.commandBuffer);

		auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

		for (auto& obj : gameObjects) {
			SimplePushConstantData push{};
			auto modelMatrix = obj.transform.mat4();
			push.transform = projectionView * modelMatrix;
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}