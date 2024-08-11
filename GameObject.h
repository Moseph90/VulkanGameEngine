#pragma once

#include "Model.h"

#include <glm/gtc/matrix_transform.hpp> // This helps us construct 4x4 transformation matrices
#include <memory>

namespace engine {
	struct TransformComponent {
		glm::vec3 translation{}; // Position offset
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 rotation;

		// 4 dimensions here because we have 3 spacial dimensions and 1 for homogenous coordinates
		// We use the current property values of the transform component to construct a combined
		// 4x4 affined transformation matrix which is translate * Ry * Rx * Rz * scale transformation
		// Rotation convention uses tait-bryan angles with axis order Y(1), X(2), Z(3) in that order
		// More information: https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	class GameObject {
	public:
		using id_t = unsigned int;

		static GameObject createGameObject() {
			static id_t currentId = 0;
			return GameObject{ currentId++ };
		}

		GameObject(const GameObject&) = delete;
		GameObject &operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject &operator=(GameObject&&) = default;

		id_t getId() const { return id; }

		std::shared_ptr<Model> model{};
		glm::vec3 color{};
		TransformComponent transform{};


	private:
		GameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}