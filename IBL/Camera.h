#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CCamera {
public:
	CCamera(glm::dvec2& mousePosition,
			glm::dvec3& vPosition = glm::dvec3(0.0, 0.0, 3.0),
			glm::dvec3& vFront = glm::dvec3(0.0, 0.0, -1.0),
			glm::dvec3& vUp = glm::dvec3(0.0, 1.0, 0.0),
			GLfloat vYaw = -90.0f,
			GLfloat vPitch = 0.0f);
	~CCamera();
	void activeCamera(GLFWwindow* vpWindow);
	glm::mat4 getViewMatrix() const;
	glm::dvec3 getCameraPosition() const;
	void setFrameTime(GLfloat vFrameTime) { m_FrameTime = vFrameTime; };
	
private:
	static glm::dvec3 m_CameraPosition;
	static glm::dvec3 m_CameraFront;
	static glm::dvec3 m_CameraUp;
	static glm::dvec3 m_CameraRight;
	static glm::dvec3 m_WorldUp;
	static glm::dvec2 m_LastMouse;
	static double m_MovementSpeed;
	static double m_MouseSensitivity;
	static GLfloat m_Yaw;
	static GLfloat m_Pitch;
	static bool m_RoamBegin;
	static GLfloat m_FrameTime;

	static void __updateCameraVectors();
	static void __callbackKeyboard(GLFWwindow* vpWindow, int vKey, int vScancode, int vAction, int vMode);
	static void __callbackMouse(GLFWwindow* vpWindow, double vPosX, double vPosY);
};