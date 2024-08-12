#include "Application.h"
#include "RenderSystem.h"
#include "Camera.h"
#include "InputController.h"
#include "Buffer.h"

// libs
#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <chrono>
#include <iostream>

namespace engine {
    // This serves a similar purpose as the simple push constant data
    // using it as a way to pass and read data to the pipeline shaders
    struct GlobalUbo {
        glm::mat4 projectionView{ 1.0f };
        glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, -3.0f, -1.0f));
    };

    // This stuff is getting the scroll wheel behaviour from the user
    double scroll{ 0 };
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        scroll = yoffset;
    }

	Application::Application() {
		loadGameObjects();			// This uses the Model class to take vertex 
                                    // data from the CPU and copy it into the GPU
        glfwSetScrollCallback(window.getGLFWwindow(), scroll_callback);
    }

	Application::~Application() {}

	void Application::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(
                device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,     // The value that dictate how many frames can
                                                        // be submitted for rendering simultaneously.
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);   // Not using host coherent so that we can selectively flush
            uboBuffers[i]->map();
        }

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
            cameraController.moveInPlaneXZ(frameTime, viewerObject, scroll);
            scroll = 0;

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
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera
                };
                
                // update in memory
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();        // Manually flush memory to the GPU

                // draw calls will be recorded
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(frameInfo, gameObjects);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}
		vkDeviceWaitIdle(device.device());
	}

    void Application::loadGameObjects() {

        std::shared_ptr<Model> model = Model::createModelFromFile(device, "TestModels/flat_vase.obj");
        auto gameObject = GameObject::createGameObject();
        gameObject.model = model;
        gameObject.transform.translation = { 2.0f, 0.5f, 2.5f };
        gameObject.transform.scale = {3.0f, 2.0f, 3.0f};
        gameObjects.push_back(std::move(gameObject));

        model = Model::createModelFromFile(device, "TestModels/smooth_vase.obj");
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = model;
        smoothVase.transform.translation = { 0.0f, 0.5f, 2.5f };
        smoothVase.transform.scale = glm::vec3(3.0f);
        gameObjects.push_back(std::move(smoothVase));

        model = Model::createModelFromFile(device, "TestModels/colored_cube.obj");
        auto coloredCube = GameObject::createGameObject();
        coloredCube.model = model;
        coloredCube.transform.translation = { -2.0f, 0.0f, 2.5f };
        coloredCube.transform.scale = glm::vec3(0.5f);
        gameObjects.push_back(std::move(coloredCube));
    }
}