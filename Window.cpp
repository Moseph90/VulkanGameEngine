#include "Window.h"
#include <stdexcept>
#include <iostream>

namespace engine {
	Window::Window(int w, int h, std::string name) : width(w), height(h), windowName(name) {
		initWindow();
	}

	Window::~Window() {
		glfwDestroyWindow(window);						//We let GLFW handle destroying the window.
		glfwTerminate();								//We tell GLFW to terminate. This is a GLFW function.
	}

	void Window::initWindow() {
		glfwInit();										//Initialize GLFW library
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	//Tell GLFW not to use its default openGL context when creating the window.
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);		//Window is resizeable.

		// Here we create the window, we must pass a C style string for the name and the 4th parameter 
		// is for fullscreen, which we will leave empty. The last parameter is for openGL which we do 
		// not need and so we pass a null pointer
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(window);			//We ask GLFW if the window should close or not, we use this Application.h
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void Window::frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto tempWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		tempWindow->frameBufferResized = true;
		tempWindow->width = width;
		tempWindow->height = height;
	}
}