#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible camera movement options
enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

//// Original Camera euler angle values
//const float YAW = -90.0f;
//const float PITCH = 0.0f;
//const float ROLL = 0.0f;
//const float ZOOM = 45.0f;

class Camera
{
public:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	// Euler Angles
	float yaw;
	float pitch;
	float roll;

	// Camera options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

	Camera(glm::vec3 _position, float _yaw, float _pitch);

	void Move(CameraMovement _movement, float _deltaTime);
	void Rotate(float _xOffset, float _yOffset);

	glm::mat4 GetViewMatrix() { return glm::lookAt(position, position + front, up); }

private:
	void updateCameraVectors();
};
#endif