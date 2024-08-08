//**********************************************************************
// The purpose of this class is to be able to take vertex data created
// by or read in a file on the CPU and then allocate the memory and copy
// the data over to our device GPU so it can be rendered efficiently
//**********************************************************************

#pragma once

#include "Device.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>

#include <vector>

namespace engine {
	class Model {
	private:
		Device &device;
		VkBuffer vertexBuffer;				// In Vulkan, the buffer and it's assigned memory are
		VkDeviceMemory vertexBufferMemory;	// two separate objects so we can have more control
		uint32_t vertexCount;

		struct Vertex;
		void createVertexBuffers(const std::vector<Vertex> &vertices);
	public:
		struct Vertex {
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		Model(Device &tempDevice, const std::vector<Vertex> &vertices);
		~Model();

		// We must delete the copy constructors because the Model 
		// class manages the Vulkan buffer and memory objects
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = default;
		Model& operator=(Model&&) = default;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	};
}