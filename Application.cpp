#include "Application.h"
#include "RenderSystem.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace engine {

	Application::Application() {
		loadGameObjects();			// This uses the Model class to take vertex data from the CPU and copy it into the GPU
	}

	Application::~Application() {}

	void Application::run() {
		RenderSystem renderSystem{ device, renderer.getSwapChainRenderPass() };

		while (!window.shouldClose()) {			//This GLFW function checks for and process any 
			glfwPollEvents();					//events that occur in the window such as key
												//strokes or mouse clicks. This loop just asks
												//GLFW to continuously check while it's open 
			
			// The beginFrame function in Renderer will return
			// a nullptr if the swap chain needs to be created
			if (auto commandBuffer = renderer.beginFrame()) {
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(commandBuffer, gameObjects);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}										
		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects() {
		// Here we color each vertex of the triangle a different 
		// color and the interpolation takes care of the rest.
		std::vector<Model::Vertex> vertices {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		auto model = std::make_shared<Model>(device, vertices);

		auto triangle = GameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2D.translation.x = 0.2f;
		triangle.transform2D.scale = { 2.0f, 0.5f };
		triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}
}