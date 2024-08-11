#include "InputController.h"
#include <stdexcept>
#include <iostream>


namespace engine {
	InputController::InputController(GLFWwindow* tempWindow) {
		this->window = tempWindow;
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		else throw std::runtime_error("Raw mouse motion not supported");
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void InputController::moveInPlaneXZ(float dt, GameObject& gameObject, int scroll) {
		glm::vec3 mouseMotion = updateMouseMotion();
		glm::vec3 rotate{ 0.0f };
		/*if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y--;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y++;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x--;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x++;*/

		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		if (state == GLFW_PRESS) {
			rotate.x += mouseMotion.y;
			rotate.y += mouseMotion.x;
		}

		
		// Make sure that the rotate vector is greater than 0 so that we don't normalize 0
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * (glm::normalize(rotate)) * 1.25f;
		}

		// Limit the range of the X rotation of the game object
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);

		// Here we just prevent repeated spinning in one direction causing the value to overflow
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
		
		// Handle the translation move actions
		float yaw = gameObject.transform.rotation.y;
		float pitch = gameObject.transform.rotation.x;
		const glm::vec3 forwardDir{ sin(yaw), 0.0f, cos(yaw) };								//Forward direction
		const glm::vec3 rightDir{ forwardDir.z, 0.0f, -forwardDir.x };						//Right direction
		const glm::vec3 upDir{ sin(pitch) * sin(yaw), cos(pitch), sin(pitch) * cos(yaw)};	//Up direction
		const glm::vec3 zoomDir{ cos(pitch) * sin(yaw), -sin(pitch), cos(pitch) * cos(yaw) };//Relative forward

		// If we press the keys, set the moveDir variable to the appropriate values
		glm::vec3 moveDir{ 0.0f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
		mouseMotion = glm::normalize(mouseMotion);
		glm::vec3 moveForward{ 0.0f };
		if (state == GLFW_PRESS) {
			if (mouseMotion.x > 0)
				moveDir -= rightDir;
			else if (mouseMotion.x < 0)
				moveDir += rightDir;
			if (mouseMotion.y > 0)
				moveDir += upDir;
			else if (mouseMotion.y < 0)
				moveDir -= upDir;
		}
		if (scroll > 0) {
			moveForward += zoomDir;
		}
		else if (scroll < 0)
			moveForward -= zoomDir;

		// Make sure that the moveDir vector is greater than 0 so that we don't normalize 0
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
		if (glm::dot(moveForward, moveForward) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveForward) * 3.0f;
		}
	}

	glm::vec3 InputController::updateMouseMotion() {
		// Get current cursor position
		glfwGetCursorPos(window, &currX, &currY);

		// Calculate the motion (delta)
		double deltaX = currX - prevX;
		double deltaY = currY - prevY;

		// Store the current position as the previous position for the next frame
		prevX = currX;
		prevY = currY;

		// Now deltaX and deltaY contain the motion of the mouse in the X and Y directions
		// You can use this as a vec2 or separately
		// For example:
		glm::vec3 mouseMotion{ deltaX, -deltaY, 1.0f };
		return mouseMotion;
	}
}