//**********************************************************************
// The purpose of this class is to be able to take vertex data created
// by or read in a file on the CPU and then allocate the memory and copy
// the data over to our device GPU so it can be rendered efficiently
//**********************************************************************

#pragma once

#include "Device.h"
#include "Buffer.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace engine {
	class Model {
	private:
		Device &device;
		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;

		// This bool is so that we can have two options remain available to us. We
		// can create a model with only vertices and no indices or one that uses both
		bool hasIndexBuffer{ false };
		std::unique_ptr<Buffer> indexBuffer;
		uint32_t indexCount;

		struct Vertex;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t> &indices);
	public:
		// In this struct, we set the attributes for each vertex to be rendered
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position
					&& color == other.color
					&& normal == other.normal
					&& uv == other.uv;
			}
		};

		// This will be used as a temporary helper object storing our vertex and index information 
		// until it can be copied over into the model's vertex and index buffer memory
		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			void loadModel(const std::string &filePath);
		};

		Model(Device &tempDevice, const Model::Builder &builder);
		~Model();

		// We must delete the copy constructors because the Model 
		// class manages the Vulkan buffer and memory objects
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = default;
		Model& operator=(Model&&) = default;

		static std::unique_ptr<Model> createModelFromFile(
			Device& device, const std::string& filePath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	};
}