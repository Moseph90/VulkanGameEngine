#include "Model.h"
#include "Utils.h"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL // This will help us hash the individual glm vec components
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <unordered_map>

namespace std {
	template <>

	// With this we can take an instance of the Vertex struct and hash it to a single 
	// value type size_t, which can then be used by an unordered map as the key
	struct hash<engine::Model::Vertex> {
		size_t operator()(engine::Model::Vertex const& vertex) const {
			size_t seed = 0;		// This will store the final hash value
			engine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace engine {
	Model::Model(Device &tempDevice, const Model::Builder &builder) : device{tempDevice}{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}
	Model::~Model() {}

	std::unique_ptr<Model> Model::createModelFromFile(
		Device& device, const std::string& filePath) {
		Builder builder{};
		builder.loadModel(filePath);
		return std::make_unique<Model>(device, builder);
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		// Total number of bytes required for our vertex buffer to store all the vertices of the model
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		// We create a stage buffer so that we can use local memory which more efficient
		// We destroy this after we're done copying it to the main vertex buffer.
		Buffer stagingBuffer{
			device,
			vertexSize,
			vertexCount,
			// This tells Vulkan that the buffer we are creating is going to be
			// used just as the source location for our memory transfer operation
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			// Host visible bit tells Vulkan that we want the allocated memory to be accessible from our
			// host (aka the CPU). This is necessary for our host to be able to to write the device memory.
			// The host coherent bit property keeps the host and device memory regions consistent with each
			// other. If this property is absent, then we are required to call vkFlushMappedMemoryRanges
			// in order to propagate changes from host to device memory.
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		// This function call creates a region of post memory mapped to device 
		// memory and sets data to the beginning of the mapped memory range.
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// This is most optimal local memory according to Vulkan

		device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	// This is identical to the createVertexBuffers function except that we are creating indices
	void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		
		// Checks
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer) return;
		assert(indexCount >= 3 && "Index count must be at least 3");

		// Total number of bytes required for our index buffer to store all the indices of the model
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer{
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	void Model::draw(VkCommandBuffer commandBuffer) {
		// If the model has an index buffer, there's no need to call both functions as
		// the draw indexed function will call whatever is bound to the command buffer
		// This includes the vertex buffer, so we only need to call one of these.
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	// This basically makes the buffers available to Vulkan
	void Model::bind(VkCommandBuffer commandBuffer) {
		// This function will record to our command buffer to bind one vertex buffer 
		// starting at binding 0 with an offset of 0 into the buffer. When we want to 
		// add multiple bindings, we can add additional elements to these arrays.
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			// Index type need to match the type of the indices vector, for smaller 
			// models you can save memory by using a smaller index type. 16 bits allow 
			// for around 65,000 vertices, whereas 32 bit allows for over 4 million.
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	// This binding description corresponds to our single vertex buffer. It will occupy the
	// first binding at index 0, the stride advances by the size of vertex bytes per vertex
	// It also deals with the color attribute that is in the Vertex struct
	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		// Arguments are in this order: 
		// 1. Location, corresponds to the location of the vertex shader
		// 2. Binding, 0 because we are interleaving everything together
		// 3. Format, specifies the data type. (3 components that are each 32 bit sin floats)
		// 4. Offset, auto calculate the byte offset of the position member in the Vertex struct
		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		return attributeDescriptions;
	}

	// Here we load in the models using tiny object loader
	void Model::Builder::loadModel(const std::string& filePath) {
		// This records the position, color, normal and texture coordinate data
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;	// Contains the index values for each face element
		std::vector<tinyobj::material_t> materials;
		std::string warn, error;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filePath.c_str())) {
			throw std::runtime_error(warn + " " + error);
		}
		vertices.clear();
		indices.clear();

		// This map will keep track of the vertices which have already been added to the
		// builder.vertices vector and store the position at which the vertex was originally added
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			// Loop through each face element in the model getting the index values
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};
				// The vertex index is the first value of the face element and says
				// what position value to use. Index values are optional and a negative
				// value indicates that no index was provided. If one is we continue
				if (index.vertex_index >= 0) {
					// Each vertex has 3 values that are tightly packed in the attrib.vertices
					// array. To read the corresponding position, we need to multiply by 3 and
					// then add 0 for the initial component, followed by 1 and 2 for Z and Y. 
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};
					// We use the last index because color attributes are optional
					// and this is a convenient way to check that a color has been
					// provided and the index is in bounds. In some formats, the
					// RGB information will be right after the last vertex position
					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				// UVs only have two values
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}
				// If the vertex is new, we add it to the unique vertices map
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				// With this we add the position of the 
				// vertex to the builder's indices vector
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}