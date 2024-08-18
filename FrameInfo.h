// ***************************************************************
// This class wraps all frame relavent data into a single struct
// which can then be easily provided to any systems funciton calls
// ***************************************************************

#pragma once

#include "Camera.h"

#include <vulkan/vulkan.h>

namespace engine {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
	};
}