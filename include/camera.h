#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// camera movement options
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// default camera values
const float YAW			= -90.0f;
const float PITCH		= 0.0f;
const float SPEED		= 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM		= 45.0f;

// cameracontroller class
class Camera
{
public:
	// constructor with vectors
	Camera(int width, int height, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	// constructor with scalar values
	Camera(int width, int height, float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	// setters
	void setLastX(float lastX);
	void setLastY(float lastY);
	void setFirstMouse(bool isFirstMouse);

	// returns the view matrix calculated using Euler angles and LookAt matrix
	glm::mat4 getViewMatrix() const;

	// processes input received from any keyboard-like input system.
	// accepts input parameter in the form of camera defined ENUM
	void processKeyboard(Camera_Movement direction, float deltaTime);

	// processes input received from a mouse input system. expects the offset value
	// in both the x and y direction
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	// processes input received from a mouse scroll-wheel event. only requires
	// input on the vertical wheel-axis
	void processMouseScroll(float yoffset);

	// invert pitch
	void invertPitch();

	// mouse handlers
	void handleMousePosition(float xpos, float ypos, GLboolean constrainPitch = true);
	void handleMouseScroll(float yoffset);

	void setEnabled(bool enabled);
	bool isEnabled() const;

	glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 200.0f) const;

private:
	// width of window
	int width_;
	// height of window
	int height_;
	// camera x,y position center
	float lastX_;
	float lastY_;
	// first mouse movement
	bool isFirstMouse_ = true;

	bool isEnabled_ = true;

	// camera attributes
	glm::vec3 position_;
	glm::vec3 front_;
	glm::vec3 up_;
	glm::vec3 right_;
	glm::vec3 worldUp_;

	// euler angles
	float yaw_;
	float pitch_;

	// camera options
	float movementSpeed_;
	float mouseSensitivity_;
	float zoom_;
private:
	// calculates the front vector from the Camera's (updated) Euler angles
	void updateCameraVectors();
};

#endif