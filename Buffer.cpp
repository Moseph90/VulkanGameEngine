#include "Buffer.h"

// std
#include <cassert>
#include <cstring>

namespace engine {

    // Returns the minimum instance size required to be compatible with devices minOffsetAlignment
    // Parameters:  instanceSize The size of an instance, minOffsetAlignment The minimum required alignment, 
    //              in bytes, for the offset member (egminUniformBufferOffsetAlignment)
    VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    Buffer::Buffer(
        Device& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : device{ device },
        instanceSize{ instanceSize },
        instanceCount{ instanceCount },
        usageFlags{ usageFlags },
        memoryPropertyFlags{ memoryPropertyFlags } {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    Buffer::~Buffer() {
        unmap();
        vkDestroyBuffer(device.device(), buffer, nullptr);
        vkFreeMemory(device.device(), memory, nullptr);
    }

    // Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
    // Size represents the size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
    // buffer range. Offset represents the byte offset from beginning. Return vkMapmemory result.
    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(device.device(), memory, offset, size, 0, &mapped);
    }

    // Unmap a mapped memory range. We don't need to return a result as vkUnmapMemory can't fail
    void Buffer::unmap() {
        if (mapped) {
            vkUnmapMemory(device.device(), memory);
            mapped = nullptr;
        }
    }

    // Copies the specified data to the mapped buffer. Default value writes whole buffer range
    // Basically, we take the vertices data we get from the Model class and copy it into the host
    // mapped memory region. Since we set the property in the Model class to coherent bit, the host 
    // memory will automatically be flushed to update the device memory.
    void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        }
        else {
            char* memOffset = (char*)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    // Here we flush a memory range of the buffer to make it visible to the device
    // Size represents the size of the memory range to flush. Pass VK_WHOLE_SIZE to 
    // flush the complete buffer range. Offset represents the byte offset from beginning
    // The vkFlushmappedMemoryRanges call will give us a VkResult to return.
    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};           
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(device.device(), 1, &mappedRange);
    }

    // Invalidate a memory range of the buffer to make it visible to the host
    // size parameter is the size of the memory range to invalidate. Pass 
    // VK_WHOLE_SIZE to invalidate the complete buffer range.
    // This returns a VkResult of the invalidate call.
    // Note that this is only required for non-coherent memory
    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(device.device(), 1, &mappedRange);
    }

    // Create a buffer info descriptor returning VkDescriptorBufferInfo 
    // using the size of the memory range and byte offset from the beginning
    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
            buffer,
            offset,
            size,
        };
    }

    // Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
    // parameters are data Pointer to the data to copy and index used in offset calculation
    void Buffer::writeToIndex(void* data, int index) {
        writeToBuffer(data, instanceSize, index * alignmentSize);
    }

    // Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
    VkResult Buffer::flushIndex(int index) { 
        return flush(alignmentSize, index * alignmentSize); 
    }

    // Create a buffer info descriptor then return
    // VkDescriptorBufferInfo for instance at index
    VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
    }

    // Invalidate a memory range of the buffer to make it visible to the host
    // This is only required for non-coherent memory. The parameter index 
    // specifies the region to invalidate: index * alignmentSize
    // then return the VkResult of the invalidate call
    VkResult Buffer::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }
}