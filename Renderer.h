//********************************************************************
// This class creates the swapchain, command buffers and draw frame.
// This class is used by the Application class to bring everything
// together. It also is responsible for creating a Device instance as
// well as memory allocation and management for the command buffers.
//********************************************************************

#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Window.h"

// std
#include <memory>
#include <cassert>

namespace engine {

	class Renderer {
	private:
		Window& window;
		Device& device;

		std::unique_ptr<SwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex{ 0 };
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

	public:
		Renderer(Window &tempWindow, Device &tempDevice);
		~Renderer();

		Renderer(const Renderer&) = delete;				//Delete copy constructors
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex]; 
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
			return currentFrameIndex;
		}
	};
}