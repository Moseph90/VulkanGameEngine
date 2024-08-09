#include "Application.h"
#include "RenderSystem.h"
#include "Camera.h"
#include "InputController.h"

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>

namespace engine {

	Application::Application() {
		loadGameObjects();			// This uses the Model class to take vertex data from the CPU and copy it into the GPU
	}

	Application::~Application() {}

	void Application::run() {
		RenderSystem renderSystem{ device, renderer.getSwapChainRenderPass() };
        Camera camera{};
        //camera.setViewDirection(glm::vec3{ 0.0f }, glm::vec3{ 0.5f, 0.0f, 1.0f });
        camera.setViewTarget(glm::vec3{-1.0f, -2.0f, 2.0f }, glm::vec3{0.0f, 0.0f, 2.5f});

        // This game object has no model and will not be rendered 
        // but is just used to store the camera's current state
        auto viewerObject = GameObject::createGameObject();
        InputController cameraController{ window.getGLFWwindow()};

        // Here we are creating a chrono object so that we can implement time
        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {			//This GLFW function checks for and process any 
			glfwPollEvents();					//events that occur in the window such as key
												//strokes or mouse clicks. This loop just asks
												//GLFW to continuously check while it's open 
            
            auto newTime = std::chrono::high_resolution_clock::now();

            // This is the time from the last loop till now
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            // Upadate current time so that can contiuously keep track of frameTime
            currentTime = newTime;

            // This will update the view object's transform component based on the keyboard 
            // input proportional to the amount of time elapsed since the last frame.
            cameraController.moveInPlaneXZ(frameTime, viewerObject);

            // We update our camera object using the new state of the view object
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            
            float aspect = renderer.getAspectRatio();
                                                
            // We put this here so that the projection matrix will always 
            // be up to date with the current aspect ratio of the window

            //We removed this one in favor of perspective
            //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1); 
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

			// The beginFrame function in Renderer will return
			// a nullptr if the swap chain needs to be created
			if (auto commandBuffer = renderer.beginFrame()) {
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}										
		vkDeviceWaitIdle(device.device());
	}

    std::unique_ptr<Model>  createCubeModel(Device& device, glm::vec3 offset) {
        std::vector<Model::Vertex> vertices{

            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

        };
        for (auto& v : vertices) {
            v.position += offset;
        }
        return std::make_unique<Model>(device, vertices);
    }
    void Application::loadGameObjects() {
        std::shared_ptr<Model> model = createCubeModel(device, { 0.0f, 0.0f, 0.0f });

        auto cube = GameObject::createGameObject();
        cube.model = model;
        cube.transform.translation = { 0.0f, 0.0f, 2.5f };
        cube.transform.scale = { 0.5f, 0.5f, 0.5f };
        gameObjects.push_back(std::move(cube));
    }
}