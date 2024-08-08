//********************************************************************
// This class creates and instance of Window.h and calls it's funcitons
// This is the actual application that runs the program. This is where
// the entire program stems from and if you follow all of the object
// creations as well as the function calls, you can piece together how
// the entire program works. The order of objects created here in the
// header file are also very important to the functionality. They are
// created even before the constructor body is executed.
//********************************************************************

#pragma once

#define WINDOW_WIDTH 3200					//Change these to alter the default window size
#define WINDOW_HEIGHT 1800

#include "Device.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Window.h"

// std
#include <memory>

namespace engine {

	class Application {
	private:
		Window window{ WIDTH, HEIGHT, "Cobra Engine" };		//Lets create an instance of the Window class with the proper parameters
															//This will also create a window for us as that handled in the Window class
		Device device{ window };
		Renderer renderer{ window, device };

		std::vector<GameObject> gameObjects;

		void loadGameObjects();

	public:
		static constexpr int WIDTH{ WINDOW_WIDTH };			//We do this so that these variables are accessable to other parts of the program
		static constexpr int HEIGHT{ WINDOW_HEIGHT };
		
		Application();
		~Application();

		Application(const Application&) = delete;				//Delete copy constructors
		Application& operator=(const Application&) = delete;

		void run();
	};
}