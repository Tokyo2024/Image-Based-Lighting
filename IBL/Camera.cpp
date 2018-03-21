#include "Camera.h"

CCamera::CCamera(glm::dvec2& vMousePosition, glm::dvec3& vPosition, glm::dvec3& vFront, glm::dvec3& vUp, GLfloat vYaw, GLfloat vPitch)
{
	m_LastMouse = vMousePosition;
	m_CameraPosition = vPosition;
	m_CameraFront = vFront;
	m_Yaw = vYaw;
	m_Pitch = vPitch;
	__updateCameraVectors();
}

CCamera::~CCamera()
{
}

//************************************
//Function:
glm::mat4 CCamera::getViewMatrix() const
{
	return glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraFront, m_CameraUp);
}

//*********************************************************************************
//FUNCTION:
glm::dvec3 CCamera::getCameraPosition() const
{
	return m_CameraPosition;
}

//************************************
//Function:
void CCamera::activeCamera(GLFWwindow* vWindow)
{
	glfwSetKeyCallback(vWindow, __callbackKeyboard);
	glfwSetCursorPosCallback(vWindow, __callbackMouse);
}

//************************************
//Function:
void CCamera::__updateCameraVectors()
{
	glm::vec3 Front;
	Front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	Front.y = sin(glm::radians(m_Pitch));
	Front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

	m_CameraFront = glm::normalize(Front);
	m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
	m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
}

//************************************
//Function:
void CCamera::__callbackKeyboard(GLFWwindow* vWindow, int vKey, int vScancode, int vAction, int vMode)
{
	if (vKey == GLFW_KEY_W)
		m_CameraPosition += m_MovementSpeed * m_FrameTime * m_CameraFront;
	if (vKey == GLFW_KEY_S)
		m_CameraPosition -= m_MovementSpeed * m_FrameTime * m_CameraFront;
	if (vKey == GLFW_KEY_A)
		m_CameraPosition -= m_MovementSpeed * m_FrameTime * glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
	if (vKey == GLFW_KEY_D)
		m_CameraPosition += m_MovementSpeed * m_FrameTime * glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
	if (vKey == GLFW_KEY_Q)
		m_CameraPosition += m_MovementSpeed * m_FrameTime * glm::normalize(m_CameraUp);
	if (vKey == GLFW_KEY_E)
		m_CameraPosition -= m_MovementSpeed * m_FrameTime * glm::normalize(m_CameraUp);
}

//************************************
//Function:
void CCamera::__callbackMouse(GLFWwindow* vWindow, double vPosX, double vPosY)
{
	if (m_RoamBegin)
	{
		m_LastMouse.x = vPosX;
		m_LastMouse.y = vPosY;
		m_RoamBegin = false;
	}

	GLfloat OffsetX = (GLfloat)(vPosX - m_LastMouse.x);
	GLfloat OffsetY = (GLfloat)(m_LastMouse.y - vPosY);
	m_LastMouse.x = vPosX;
	m_LastMouse.y = vPosY;

	OffsetX *= (GLfloat)m_MouseSensitivity;
	OffsetY *= (GLfloat)m_MouseSensitivity;

	m_Yaw += OffsetX;
	m_Pitch += OffsetY;

	m_Pitch = m_Pitch > 89.0f ? 89.0f : m_Pitch;
	m_Pitch = m_Pitch < -89.0f ? -89.0f : m_Pitch;

	__updateCameraVectors();
}

glm::dvec3 CCamera::m_CameraPosition;
glm::dvec3 CCamera::m_CameraFront;
glm::dvec3 CCamera::m_CameraUp;
glm::dvec3 CCamera::m_CameraRight = glm::dvec3(0.0);
glm::dvec3 CCamera::m_WorldUp = glm::dvec3(0.0, 1.0, 0.0);
glm::dvec2 CCamera::m_LastMouse;
double	   CCamera::m_MovementSpeed = 12.0;
double	   CCamera::m_MouseSensitivity = 0.2;
GLfloat	   CCamera::m_Yaw;
GLfloat	   CCamera::m_Pitch;
GLfloat	   CCamera::m_FrameTime;
bool       CCamera::m_RoamBegin = true;