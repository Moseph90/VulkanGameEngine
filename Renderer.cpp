#include "Renderer.h"

// std
#include <array>
#include <stdexcept>

namespace engine {

	Renderer::Renderer(Window& tempWindow, Device& tempDevice) : window{ tempWindow }, device{ tempDevice } {
		recreateSwapChain();			// This goes into Pipeline and creates a pipeline object using our unique pointer

		// In Vulkan, we're not able to execute commands directly with function calls. We're first required
		// to record them to a command buffer and then submit the buffer to a device queue to be executed.
		// The advantage is that it allows a sequence of commands to be recoded once and reused for multiple
		// frames. Unlike OpenGL where draw commands would need to be repeated for every frame.
		createCommandBuffers();
	}

	Renderer::~Renderer() {
		freeCommandBuffers();
	}

	void Renderer::recreateSwapChain() {
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
			std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
			swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*swapChain.get())) {
				throw std::runtime_error("Swap chain image (or depth) format has changed");
			}
		}
		// We'll come back to this
	}
	// Here's what a command buffer does step by step:
	// 1. We begin the render pass
	// 2. Bind our simple graphics pipeline
	// 3. Bind the model which also binds the associated vertex buffer data
	// 4. We push constants to store more information about the model. Like color and offset
	//    This would apply to every vertex in the vertex shader.
	// 5. We record a command to draw the vertex buffer data
	// 6. End the render pass
	void Renderer::createCommandBuffers() {
		// The swap chain image count will likely be 2 of 3 depending 
		// on whether the device supports triple or double buffering
		// as each command buffer will draw to a different frame buffer
		commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		
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

	void Renderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			device.device(),
			device.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	// These two functions (beginFrame and endFrame) fetch the index of the frame which we render 
	// to next. It also automatically handles all the CPU and GPU synchronization surrounding 
	// double of triple buffering. The value returned determines if this process was successful.
	VkCommandBuffer Renderer::beginFrame() {
		assert(!isFrameStarted && "Can't Call beginFrame() when already in progress");

		auto result = swapChain->acquireNextImage(&currentImageIndex);

		// We check and recreate the swap chain if the surface is no longer compatible with it
		// This will prevent crashes when resizing the window so that the surface matches
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire the next swapchain image");
		}
		isFrameStarted = true;

		auto commandBuffer = getCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//Here we try to begin recording using the Vulkan vkBeginCommandBuffer function
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Command buffer failed to begin recording");
		}
		return commandBuffer;
	}
	void Renderer::endFrame() {
		assert(isFrameStarted && "Cannot call endFrame() when frame is not in progress");

		auto commandBuffer = getCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}

		// Submit the provided command buffers to our device graphics queue while 
		// handling CPU and GPU synchronization. The command buffer will then be executed.
		// Then the swap chain will present the associated color attachment image queue
		// to the display at the appropriate time based on the present mode selected.
		auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		// VK_SUBOPTIMAL_KHR is a boolean of type VkResult that means that the swapchain no
		// longer matches the surface exactly but can still be used to present the surface.
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
			window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}
	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass() if frame is not in progress");
		assert(commandBuffer == getCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		// The first command we record is to begin a render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();		// which render pass
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);	// which frame buffer this render pass is writing
		renderPassInfo.renderArea.offset = { 0, 0 };	// This defines the area where the shader loads and stores will be
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
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// See defaultPipelineConfigInfo function in Pipeline.cpp file for info on viewports and scissors
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass() if frame is not in progress");
		assert(commandBuffer == getCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		
		vkCmdEndRenderPass(commandBuffer);			// End the render pass
	}

	// ******************************************************************************************
	// Quick note: We did not put the beginFrame and the beginSwapChainRenderPass functions in 
	// the same function. This is so that, down the line, we can easily create multiple render
	// passes for things like reflections, shadows, and post processing effects. By doing it this
	// way, we keep ourselves flexible in the features we can add to our engine later. This is 
	// also why we did not put endSwapChainRenderPass and endFrame in the same function either.
	// ******************************************************************************************
}