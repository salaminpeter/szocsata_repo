#pragma once

#include "glm\gtc\matrix_transform.hpp"


class CCamera;

class CView
{
public:
	CView(int w, int h, int x, int y);

	void InitCamera(glm::vec3 camPos, glm::vec3 lookatPos, glm::vec3 upVector);
	void InitPerspective(float fovy, float nearZ = 0.1f, float farZ = 100.f);
	void InitOrtho();
	void Activate();

	glm::vec3 GetCameraAxisInWorldSpace(int axis);
	glm::vec3 GetCameraPosition();
	glm::vec3 GetCameraLookAt();
	glm::mat4 GetProjectionView();
	glm::mat4 GetView();
	glm::vec2 GetViewPosition();
	glm::vec2 GetViewSize();
	void PositionCamera(glm::vec3 translate);
	void LookAt();
	void RotateCamera(float a);

private:
	
	int m_Width;
	int m_Height;
	int m_XPosition;
	int m_YPosition;

	CCamera* m_Camera;
};
