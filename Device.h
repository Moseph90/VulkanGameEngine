//**************************************************************************************************
// This file contains the code for identifying and selecting features for from our graphics card
// That part of the code is documented here and in the Device.cpp file for your convenience. But
// it also contains many validation layers that, to be honest I do not fully understand myself
// The full tutorial on how the validation layers work can be foud at the following address: 
// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers Also, on that same page, 
// see the 'physical devices and queue families' and 'Logical device queues' sub sections which 
// should be in the same section as the validation layers tutorial. For a more in-depth explanation, 
// see this address: https://docs.vulkan.org/spec/latest/index.html which contains detailed 
// documentation that addressed each keyword and function which you can cross reference
//**************************************************************************************************


#pragma once

#include "Window.h"

//std lib headers
#include <vector>

namespace engine {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() {
            return graphicsFamilyHasValue && presentFamilyHasValue;
        }
    };

    class Device {
     private:
          void createInstance();
          void setupDebugMessenger();
          void createSurface();
          void pickPhysicalDevice();
          void createLogicalDevice();
          void createCommandPool();

          // helper functions
          bool isDeviceSuitable(VkPhysicalDevice device);
          std::vector<const char *> getRequiredExtensions();
          bool checkValidationLayerSupport();
          QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
          void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
          void hasGflwRequiredInstanceExtensions();
          bool checkDeviceExtensionSupport(VkPhysicalDevice device);
          SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

          VkInstance instance;
          VkDebugUtilsMessengerEXT debugMessenger;
          VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
          Window &window;
          VkCommandPool commandPool;

          VkDevice device_;
          VkSurfaceKHR surface_;
          VkQueue graphicsQueue_;
          VkQueue presentQueue_;

          VkFence fence;

          const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
          const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
     
    public:
        #ifdef NDEBUG
          const bool enableValidationLayers = false;
        #else
          const bool enableValidationLayers = true;
        #endif

          Device(Window &window);
          ~Device();

          // Not copyable or movable
          Device(const Device &) = delete;
          Device &operator=(const Device &) = delete;
          Device(Device &&) = delete;
          Device &operator=(Device &&) = delete;

          VkCommandPool getCommandPool() { return commandPool; }
          VkDevice device() { return device_; }
          VkSurfaceKHR surface() { return surface_; }
          VkQueue graphicsQueue() { return graphicsQueue_; }
          VkQueue presentQueue() { return presentQueue_; }

          SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
          uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
          QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
          VkFormat findSupportedFormat(
              const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

          // Buffer Helper Functions
          void createBuffer(
              VkDeviceSize size,
              VkBufferUsageFlags usage,
              VkMemoryPropertyFlags properties,
              VkBuffer &buffer,
              VkDeviceMemory &bufferMemory);
          VkCommandBuffer beginSingleTimeCommands();
          void endSingleTimeCommands(VkCommandBuffer commandBuffer);
          void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
          void copyBufferToImage(
              VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

          void createImageWithInfo(
              const VkImageCreateInfo &imageInfo,
              VkMemoryPropertyFlags properties,
              VkImage &image,
              VkDeviceMemory &imageMemory);

          VkPhysicalDeviceProperties properties;
    };

}