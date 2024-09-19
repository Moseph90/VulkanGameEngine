// ***************************************************************
// This class wraps all frame relavent data into a single struct
// which can then be easily provided to any systems funciton calls
// ***************************************************************

#pragma once

#include "Camera.h"
#include "GameObject.h"

#include <vulkan/vulkan.h>

namespace engine {

#define MAX_LIGHTS 10 // How many lights we allow in a given scene (for performance purposes)

    struct PointLight {
        glm::vec4 position{}; // Ignore w
        glm::vec4 color{}; // w is intensity
    };

    // This serves a similar purpose as the simple push constant data
    // using it as a way to pass and read data to the pipeline shaders
    // We can pass in point lights and other data in here, as now the
    // engine has the framework to easily implement them.
    struct GlobalUbo {
        glm::mat4 projection{ 1.0f };
        glm::mat4 view{ 1.0f };
        glm::mat4 inverseView{ 1.0f };

        // These need to be aligned to 16 bytes. But it doesn't work here because
        // vec3 and vec4 are not the same size and the CPU packs it tightly and so
        // they will not be aligned to every 16 bytes. One way to fix this is to add
        // a variable such as uint32_t of padding which will be 4 bytes, this will be
        // added and then cause the vec3 and vec4 to be aligned as vec3 is 12 bytes and
        // vec4 is 16. Alternatively, we can make lightPosition a vec4 and ignore the W
        glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f }; // The 4th dimension is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights; // How many active lights
    };

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
		GameObject::Map& gameObjects;
	};
}