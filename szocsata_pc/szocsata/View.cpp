#include "stdafx.h"
#include "Camera.h"
#include "View.h"
#include "Config.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

CView::CView(int w, int h, int x, int y) : m_Width(w), m_Height(h), m_XPosition(x), m_YPosition(y)
{
	m_Camera = new CCamera;
}

void CView::TranslateCamera(const glm::vec3& t)
{
	m_Camera->TranslateCamera(t);
}

void CView::RotateCamera(float angle, const glm::vec3& axis)
{
	m_Camera->RotateCamera(angle, axis);
}

void CView::InitCamera(glm::vec3 camPos, glm::vec3 lookatPos, glm::vec3 upVector)
{
	//TODO!!!!!!!!! a renderer tilt anglet mindenkepp ujra kell szamolni!!!!!
	m_Camera->InitView(camPos, lookatPos, upVector);
}

void CView::InitPerspective(float fovy, float nearZ, float farZ)
{
	m_Camera->InitPerspective(fovy, float(m_Width) / float(m_Height), nearZ, farZ);
}

void CView::InitOrtho()
{
	m_Camera->InitOrtho(-m_Width / 2.f, m_Width / 2.f, m_Height / 2.f, -m_Height / 2.f);
}

void CView::Activate()
{
	glViewport(m_XPosition, m_YPosition, m_Width, m_Height);
}

glm::vec3 CView::GetCameraAxisInWorldSpace(int axis)
{
	return m_Camera->GetCameraAxisInWorldSpace(axis);
}


glm::vec3 CView::GetCameraPosition()
{
	return m_Camera->GetPosition();
}

glm::vec3 CView::GetCameraLookAt()
{
	return m_Camera->GetLookAt();
}

void CView::PositionCamera(glm::vec3 pos)
{
	m_Camera->SetPosition(pos);
}

glm::mat4 CView::GetProjectionView()
{
	return m_Camera->ProjectionMatrix() * m_Camera->ViewMatrix();
}

glm::mat4 CView::GetView()
{
	return m_Camera->ViewMatrix();
}

glm::vec2 CView::GetViewPosition()
{
	return glm::vec2(m_XPosition, m_YPosition);
}

glm::vec2 CView::GetViewSize()
{
	return glm::vec2(m_Width, m_Height);
}


