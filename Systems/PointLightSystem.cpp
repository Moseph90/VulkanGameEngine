#include "PointLightSystem.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <map>

namespace engine {

	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(
		Device& tempDevice, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: device{ tempDevice } {
		// This creates the layout and initializes the device object settings
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		// Right now we only have one, but we will make vector for when we add more
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		// This where we tell the pipeline about the descriptor set layouts
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

		//Push constants are a way to send very small amounts of data to our shader program
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		//This is where the device is created and the layout is checked
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr,
			&pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		//Here we use the swapchain's width and height as it may not match the window's
		Pipeline::defultPipelineConfigInfo(pipelineConfig);
		Pipeline::enableAlphaBlending(pipelineConfig);

		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.attributeDescriptions.clear();

		//Render pass describes structure and format of our frame buffer objects and their
		//attachments. For example it tells the graphics pipeline what we will be using and
		//what to expect in our output frame buffers and how they work. This way it's
		//prepared to output to a frame buffer with the correct settings
		pipelineConfig.renderPass = renderPass;

		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(
			"Shaders/PointLight.vert.spv",		//These are the files written in GLSL for the graphics
			"Shaders/PointLight.frag.spv",		//and then comipled using the compile.bat file
			device,
			pipelineConfig);
	}

	// If there is indeed a point light, copy point light to ubo
	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
		auto rotateLight = glm::rotate(glm::mat4(1.0f), frameInfo.frameTime, { 0.0f, -1.0f, 0.0f });
		
		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			assert(lightIndex < MAX_LIGHTS && "Maximum Point Lights Exceeded!");

			// Update light position
			obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.0f));

			// Copy light to ubo
			ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0f);
			ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			lightIndex += 1;
		}
		ubo.numLights = lightIndex;
	}

	void PointLightSystem::render(FrameInfo& frameInfo) {
		std::map<float, GameObject::id_t> sorted;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			// Calculate distance
			auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
			float distSquared = glm::dot(offset, offset);
			sorted[distSquared] = obj.getId();
		}

		pipeline->bind(frameInfo.commandBuffer);

		// We do this outside of the for loop (below this) 
		// because there's no need to re-bind. We only do this 
		// once and then the values in the GlobalUbo struct 
		// (in Application.cpp) can be used by all game objects 
		// without the need for re-binding
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr);

		// Iterate through the sorted lights in reverse order so that we 
		// maintain the correct transparency no matter the perspective
		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			// Use game obj id to find light object
			auto& obj = frameInfo.gameObjects.at(it->second);

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.transform.translation, 1.0f);
			push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			push.radius = obj.transform.scale.x;

			vkCmdPushConstants(
				frameInfo.commandBuffer, 
				pipelineLayout, 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
				0, 
				sizeof(PointLightPushConstants), 
				&push);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}
}