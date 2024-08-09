#include "InputController.h"

namespace engine {
	void InputController::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject) {
		glm::vec3 rotate{ 0.0f };
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y--;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y++;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x--;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x++;
		
		// Make sure that the rotate vector is greater than 0 so that we don't normalize 0
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// Limit the range of the X rotation of the game object
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);

		// Here we just prevent repeated spinning in one direction causing the value to overflow
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
		
		// Handle the translation move actions
		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.0f, cos(yaw) };				//Forward direction
		const glm::vec3 rightDir{ forwardDir.z, 0.0f, -forwardDir.x };		//Right direction
		const glm::vec3 upDir{ 0.0f, -1.0f, 0.0f };							//Up direction

		// If we press the keys, set the moveDir variable to the appropriate values
		glm::vec3 moveDir{ 0.0f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir -= upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir += upDir;

		// Make sure that the moveDir vector is greater than 0 so that we don't normalize 0
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}
}