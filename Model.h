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

		// This bool is so that we can have two options remain available to us. We
		// can create a model with only vertices and no indices or one that uses both
		bool hasIndexBuffer{ false };
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

		struct Vertex;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t> &indices);
	public:
		// In this struct, we set the attributes for each vertex to be rendered
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		// This will be used as a temporary helper object storing our vertex and index information 
		// until it can be copied over into the model's vertex and index buffer memory
		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		Model(Device &tempDevice, const Model::Builder &builder);
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