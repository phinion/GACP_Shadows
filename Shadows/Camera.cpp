#include "camera.h"

// Constructor
Camera::Camera(glm::vec3 _position, float _yaw, float _pitch)
{

	position = _position;
	yaw = _yaw;
	pitch = _pitch;

	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	movementSpeed = 2.5f;
	mouseSensitivity = 0.1f;

	updateCameraVectors();
}

// Processes keyboard input to move camera object
void Camera::Move(CameraMovement _movement, float _deltaTime)
{
	float velocity = movementSpeed * _deltaTime;
	if (_movement == FORWARD)
		position += front * velocity;
	if (_movement == BACKWARD)
		position -= front * velocity;
	if (_movement == LEFT)
		position -= right * velocity;
	if (_movement == RIGHT)
		position += right * velocity;
}

// Processes mouse input to rotate camera object
void Camera::Rotate(float _xOffset, float _yOffset)
{
	_xOffset *= mouseSensitivity;
	_yOffset *= mouseSensitivity;

	yaw += _xOffset;
	pitch += _yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	updateCameraVectors();
}

// Calculates the Front, Right and Up vectors using pitch and yaw values.
void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	// Re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, worldUp)); 
	up = glm::normalize(glm::cross(right, front));
}