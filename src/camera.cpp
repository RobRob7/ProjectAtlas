#include "Camera.h"

//--- PUBLIC ---//
// constructor with vectors
Camera::Camera(int width, int height, glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: width_(width), height_(height), m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM)
{
	lastX_ = width_ / 2.0f;
	lastY_ = width_ / 2.0f;
	m_position = position;
	m_worldUp = up;
	m_yaw = yaw;
	m_pitch = pitch;
	updateCameraVectors();
} // end constructor

// constructor with scalar values
Camera::Camera(int width, int height, float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	: width_(width), height_(height), m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM)
{
	lastX_ = width_ / 2.0f;
	lastY_ = width_ / 2.0f;
	m_position = glm::vec3(posX, posY, posZ);
	m_worldUp = glm::vec3(upX, upY, upZ);
	m_yaw = yaw;
	m_pitch = pitch;
	updateCameraVectors();
} // end constructor

// getters
float Camera::getLastX()
{
	return lastX_;
} // end of getLastX()

float Camera::getLastY()
{
	return lastY_;
} // end of getLastY()

bool Camera::getFirstMouse()
{
	return isFirstMouse_;
} // end of getFirstMouse()

// setters
void Camera::setLastX(float lastX)
{
	lastX_ = lastX;
} // end of setLastX()

void Camera::setLastY(float lastY)
{
	lastY_ = lastY;
} // end of setLastY()

void Camera::setFirstMouse(bool isFirstMouse)
{
	isFirstMouse_ = isFirstMouse;
} // end of setFirstMouse()


// returns the view matrix calculated using Euler angles and LookAt matrix
glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(m_position, m_position + m_front, m_up);
} // end of getViewMatrix()

// processes input received from any keyboard-like input system.
// accepts input parameter in the form of camera defined ENUM
void Camera::processKeyboard(Camera_Movement direction, float deltaTime)
{
	if (m_isEnabled)
	{
		float velocity = m_movementSpeed * deltaTime;
		if (direction == FORWARD)
			m_position += m_front * velocity;
		if (direction == BACKWARD)
			m_position -= m_front * velocity;
		if (direction == LEFT)
			m_position -= m_right * velocity;
		if (direction == RIGHT)
			m_position += m_right * velocity;
	}
} // end of processKeyboard()

// processes input received from a mouse input system. expects the offset value
// in both the x and y direction
void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	// only process if camera is enabled
	if (m_isEnabled)
	{
		xoffset *= m_mouseSensitivity;
		yoffset *= m_mouseSensitivity;

		m_yaw += xoffset;
		m_pitch += yoffset;

		// check pitch within bounds
		if (constrainPitch)
		{
			if (m_pitch > 89.0f)
				m_pitch = 89.0f;
			if (m_pitch < -89.0f)
				m_pitch = -89.0f;
		}

		// update m_front, m_right, and m_up vectors using updated Euler angles
		updateCameraVectors();
	}
} // end of processMouseMovement()

// processes input received from a mouse scroll-wheel event. only requires
// input on the vertical wheel-axis
void Camera::processMouseScroll(float yoffset)
{
	// only process if camera is enabled
	if (m_isEnabled)
	{
		m_zoom -= (float)yoffset;
		if (m_zoom < 1.0f)
			m_zoom = 1.0f;
		if (m_zoom > 45.0f)
			m_zoom = 45.0f;
	}
} // end of processMouseScroll()

// invert pitch
void Camera::invertPitch()
{
	m_pitch = -m_pitch;
	updateCameraVectors();
} // end of 


//--- PRIVATE ---//
// calculates the front vector from the Camera's (updated) Euler angles
void Camera::updateCameraVectors()
{
	// only process if camera is enabled
	if (m_isEnabled)
	{
		// calculate the new m_front vector
		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = glm::normalize(front);

		// re-calculate m_right and m_up vector
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}
} // end of updateCameraVectors()
