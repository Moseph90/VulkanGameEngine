#include "Application.h"

//std includes
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {

	engine::Application app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;						//GLFW macro
	}
	return EXIT_SUCCESS;							//GLFW macro
}