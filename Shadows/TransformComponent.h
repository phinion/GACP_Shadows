#ifndef _TRANSFORMCOMPONENT_H_
#define _TRANSFORMCOMPONENT_H_

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
public:
	void setPos(glm::vec3 _position){position = _position;}
	void setScale(glm::vec3 _scale) { scale = _scale; }
	void setRotation(glm::vec3 _rotation) { rotation = _rotation; }

	glm::vec3 getPos() { return position; }
	glm::vec3 getScale() { return scale; }
	glm::vec3 getRotation() { return rotation; }

	glm::mat4 getModel() {
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, position);
		model = glm::scale(model, scale);
		//havent implemented rotate yet

		return model;
	}

protected:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 rotation;
};

#endif