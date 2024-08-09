//**********************************
//This is where we create the window
//**********************************
#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>

namespace engine {
	class Window {
	private:
		std::string windowName;
		GLFWwindow* window;			//This is a GLFW variable containing a window object.

		int width;
		int height;
		bool frameBufferResized = false;

		void initWindow();
		static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);


	public:
		Window(int width, int height, std::string name);	//A contstructor for initializing the width, height and name of the window
		~Window();											//We will use these initialized values to creat the window to our liking
		//See Window.cpp for the definitions

		Window(const Window&) = delete;						//Some more clean up, just in case there are duplicate windows.
		Window& operator=(const Window&) = delete;

		bool shouldClose();									//We will use this in Application.h

		VkExtent2D getExtent() {
			return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		}
		bool wasWindowResized() const { return frameBufferResized; }
		void resetWindowResizedFlag() { frameBufferResized = false; }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		GLFWwindow* getGLFWwindow() const { return window; }
	};
}