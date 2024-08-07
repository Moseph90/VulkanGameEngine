#include "Application.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace engine {
	struct SimplePushConstantData{
		glm::mat2 transform{ 1.0f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	Application::Application() {
		loadGameObjects();				// This takes vertex data from the CPU and copies it into the GPU
		createPipelineLayout();			// This creates the layout and initializes the device object settings
		recreateSwapChain();			// This goes into Pipeline and creates a pipeline object using our unique pointer

		// In Vulkan, we're not able to execute commands directly with function calls. We're first required
		// to record them to a command buffer and then submit the buffer to a device queue to be executed.
		// The advantage is that it allows a sequence of commands to be recoded once and reused for multiple
		// frames. Unlike OpenGL where draw commands would need to be repeated for every frame.
		createCommandBuffers();
	}

	Application::~Application() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void Application::run() {

		while (!window.shouldClose()) {			//This GLFW function checks for and process any 
			glfwPollEvents();					//events that occur in the window such as key
			drawFrame();						//strokes or mouse clicks. This loop just asks
		}										//GLFW to continuously check while it's open 

		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects() {
		// Here we color each vertex of the triangle a different 
		// color and the interpolation takes care of the rest.
		std::vector<Model::Vertex> vertices {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		auto model = std::make_shared<Model>(device, vertices);

		auto triangle = GameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2D.translation.x = 0.2f;
		triangle.transform2D.scale = { 2.0f, 0.5f };
		triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}

	void Application::createPipelineLayout() {
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

	void Application::createPipeline() {
		assert(swapChain != nullptr && "Cannot create swapchain before pipeline");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		//Here we use the swapchain's width and height as it may not match the window's
		Pipeline::defultPipelineConfigInfo(pipelineConfig);

		//Render pass describes structure and format of our frame buffer objects and their
		//attachments. For example it tells the graphics pipeline what we will be using and
		//what to expect in our output frame buffers and how they work. This way it's
		//prepared to output to a frame buffer with the correct settings
		pipelineConfig.renderPass = swapChain->getRenderPass();

		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(
			"Shaders/SimpleShader.vert.spv",		//These are the files written in GLSL for the graphics
			"Shaders/SimpleShader.frag.spv",		//and then comipled using the compile.bat file
			device,
			pipelineConfig);
	}
	void Application::recreateSwapChain() {
		auto extent = window.getExtent();

		while (extent.height == 0 || extent.width == 0) {
			extent = window.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device.device());
		//swapChain.reset(nullptr);

		if (swapChain == nullptr) {
			swapChain = std::make_unique<SwapChain>(device, extent);
		}
		else {
			swapChain = std::make_unique<SwapChain>(device, extent, std::move(swapChain));
			if (swapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		
		createPipeline();
	}
	// Here's what a command buffer does step by step:
	// 1. We begin the render pass
	// 2. Bind our simple graphics pipeline
	// 3. Bind the model which also binds the associated vertex buffer data
	// 4. We push constants to store more information about the model. Like color and offset
	//    This would apply to every vertex in the vertex shader.
	// 5. We record a command to draw the vertex buffer data
	// 6. End the render pass
	void Application::createCommandBuffers() {
		// The swap chain image count will likely be 2 of 3 depending 
		// on whether the device supports triple or double buffering
		// as each command buffer will draw to a different frame buffer
		commandBuffers.resize(swapChain->imageCount());	//Make sure our commandBuffers vector is
														//the same size as the swapChain image count.
		VkCommandBufferAllocateInfo allocInfo{};		//Here we allocate our command buffers

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

		// There are two types of command buffers, primary and secondary. Primary can be submitted to a
		// queue for execution, but cannot be called by other command buffers. Whereas secondary buffers
		// cannot be submitted to queues but can be called by other command buffers
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data())
			!= VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer");
		}
	}

	void Application::freeCommandBuffers() {
		vkFreeCommandBuffers(
			device.device(), 
			device.getCommandPool(), 
			static_cast<uint32_t>(commandBuffers.size()), 
			commandBuffers.data());
		commandBuffers.clear();
	}

	void Application::renderGameObjects(VkCommandBuffer commandBuffer) {
		pipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			obj.transform2D.rotation = glm::mod(obj.transform2D.rotation + 0.01f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = obj.transform2D.translation;
			push.color = obj.color;
			push.transform = obj.transform2D.mat2();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}

	// This function fetches the index of the frame which we render to next. It also automatically
	// handles all the CPU and GPU synchronization surrounding double of triple buffering. The
	// result value returned determines if this process was successful.
	void Application::drawFrame() {
		uint32_t imageIndex;
		auto result = swapChain->acquireNextImage(&imageIndex);

		// We check and recreate the swap chain if the surface is no longer compatible with it
		// This will prevent crashes when resizing the window so that the surface matches
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire the next swapchain image");
		}
		recordCommandBuffers(imageIndex); 

		// Submit the provided command buffers to our device graphics queue while 
		// handling CPU and GPU synchronization. The command buffer will then be executed.
		// Then the swap chain will present the associated color attachment image queue
		// to the display at the appropriate time based on the present mode selected.
		result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		// VK_SUBOPTIMAL_KHR is a boolean of type VkResult that means that the swapchain no
		// longer matches the surface exactly but can still be used to present the surface.
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
			window.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image");
		}
	}

	void Application::recordCommandBuffers(int imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//Here we try to begin recording using the Vulkan vkBeginCommandBuffer function
		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo)
			!= VK_SUCCESS) {
			throw std::runtime_error("Command buffer failed to begin recording");
		}
		// The first command we record is to begin a render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();		// which render pass
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(imageIndex);	// which frame buffer this render pass is writing
		renderPassInfo.renderArea.offset = { 0,0 };	// This defines the area where the shader loads and stores will be
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();	// Use swap chain extent instead of window 
																			// extent because on high density displays
																			// the swap chain extent may actually be 
																			// larger than our windows.
		// Next we need to set the clear values. This corresponds to what we want
		// the initial values of our frame buffer attachments to be cleared to.
		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 0.1f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Now we record to our command buffer to begin this render pass
		// the last parameter signals that all the commands will be directly
		// embedded in the primary command buffer itself and that no secondary
		// command buffers will be used. Shorthand for this is INLINE.
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//See defaultPipelineConfigInfo function in Pipeline.cpp file for info on viewports and scissors
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);			// End the render pass

		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}
	}
}