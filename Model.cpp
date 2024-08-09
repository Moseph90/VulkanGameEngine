#include "Model.h"
#include <cassert>

namespace engine {
	Model::Model(Device &tempDevice, const Model::Builder &builder) : device{tempDevice}{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}
	Model::~Model() {
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(device.device(), indexBuffer, nullptr);
			vkFreeMemory(device.device(), indexBufferMemory, nullptr);
		}
	}
	void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		// Total number of bytes required for our vertex buffer to store all the vertices of the model
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		// We create a stage buffer so that we can use local memory which more efficient
		// We destroy this after we're done copying it to the main vertex buffer.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.createBuffer(
			bufferSize,
			// This tells Vulkan that the buffer we are creating is going to be
			// used just as the source location for our memory transfer operation
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			// Tells our device what the buffer will be used for
			// Host visible bit tells Vulkan that we want the allocated memory to be accessible from our
			// host (aka the CPU). This is necessary for our host to be able to to write the device memory.
			// The host coherent bit property keeps the host and device memory regions consistent with each
			// other. If this property is absent, then we are required to call vkFlushMappedMemoryRanges
			// in order to propagate changes from host to device memory.
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		// This function creates a region of post memory mapped to device 
		// memory and sets data to the beginning of the mapped memory range.
		vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);

		// This one takes the vertices data and copies it into the host mapped memory region. Since we set
		// the property in device.creatBuffer(above) to coherent bit, the host memory will automatically be
		// flushed to update the device memory. Without that property we would have to call a function.
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // This is most optimal local memory according to Vulkan
			vertexBuffer,
			vertexBufferMemory);

		device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
	}

	// This is identical to the createVertexBuffers function except that we are creating indices
	void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) return;

		assert(indexCount >= 3 && "Index count must be at least 3");

		// Total number of bytes required for our vertex buffer to store all the vertices of the model
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		// We create a stage buffer so that we can use local memory which more efficient
		// We destroy this after we're done copying it to the main vertex buffer. This
		// will give us a massive boost in performance when our models get more complex.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// This gets created on the GPU
		device.createBuffer(
			bufferSize,
			// This tells Vulkan that the buffer we are creating is going to be
			// used just as the source location for our memory transfer operation
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			// Tells our device what the buffer will be used for
			// Host visible bit tells Vulkan that we want the allocated memory to be accessible from our
			// host (aka the CPU). This is necessary for our host to be able to to write the device memory.
			// The host coherent bit property keeps the host and device memory regions consistent with each
			// other. If this property is absent, then we are required to call vkFlushMappedMemoryRanges
			// in order to propagate changes from host to device memory.
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		// This function creates a region of post memory mapped to device 
		// memory and sets data to the beginning of the mapped memory range.
		vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);

		// This one takes the vertices data and copies it into the host mapped memory region. Since we set
		// the property in device.creatBuffer(above) to coherent bit, the host memory will automatically be
		// flushed to update the device memory. Without that property we would have to call a function.
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // This is most optimal local memory according to Vulkan
			indexBuffer,
			indexBufferMemory);

		device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
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
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			// Index type need to match the type of the indices vector, for smaller 
			// models you can save memory by using a smaller index type. 16 bits allow 
			// for around 65,000 vertices, whereas 32 bit allows for over 4 million.
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0; // This corresponds to the location in the vertex shader
		
		// This specifies the data type that we have three components that are each 32 bit sin floats
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;

		// This will automatically calculate the byte offset of the position member in the Vertex struct
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		// Binding remains 0 for the color because we are interleaving the color and position together
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1; // This corresponds to the location in the fragment shader
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		
		// This will automatically calculate the byte offset of the color member in the Vertex struct
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
}