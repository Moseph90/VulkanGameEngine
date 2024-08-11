// **************************************************************
// This class if for taking user input and applying it the engine
// **************************************************************

#pragma once

#include "GameObject.h"
#include "Window.h"

namespace engine {
	class InputController {
	private:
        GLFWwindow* window;
        double prevX{ 0 }, prevY{ 0 };
        double currX{ 0 }, currY{ 0 };

        glm::vec3 updateMouseMotion();
	
	public:
		struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;

            // These ones are for arrow keys
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
		};
        InputController(GLFWwindow* tempWindow);

        // This function will move a game object with the controls being 
        // relative to the direction the object is facing within the XZ plane
        void moveInPlaneXZ(float dt, GameObject& gameObject, int scroll);

        KeyMappings keys{};
        float moveSpeed{ 3.0f };
        float lookSpeed{ 1.5f };
	};
}