#pragma once

#define GLM_FORCE_RADIANS				// All GLM functions will expect angles in radians 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM will expect or depth buffer values to range from 0 - 1
#include <glm/glm.hpp>

namespace engine {
	class Camera {
	private:
		glm::mat4 projectionMatrix{ 1.0f };
		glm::mat4 viewMatrix{ 1.0f };		//This is for the camera transform
		glm::mat4 inverseViewMatrix{ 1.0f };

	public:
		void setOrthographicProjection(
			float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);
		
		void setViewDirection(
			glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
		
		// This is useful if you want to have the camera locked to a specific point in space
		void setViewTarget(
			glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
		
		// This will use Euler angles to specify the orientation of the camera
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }
		const glm::mat4& getInverseView() const { return inverseViewMatrix; }
		const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }
	};
}