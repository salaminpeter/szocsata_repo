#include "stdafx.h"
#include "Camera.h"

#include <glm/gtx/rotate_vector.hpp>


void CCamera::InitPerspective(float fovY, float ratio, float nearZ, float farZ)
{
	m_ProjectionMatrix = glm::perspective(glm::radians(fovY), ratio, nearZ, farZ);
}

void CCamera::InitOrtho(float left, float right, float top, float bottom)
{
	m_ProjectionMatrix = glm::ortho(left, right, bottom, top, 0.0f, 100.0f);
}

void CCamera::InitView(const glm::vec3& position, const glm::vec3& lookAtPosition, const glm::vec3& upVector)
{
	m_ViewMatrix = glm::lookAt(position, lookAtPosition, upVector);
	m_InverseViewMatrix = glm::inverse(m_ViewMatrix);
}

glm::vec3 CCamera::GetPosition()
{
	return glm::vec3(m_InverseViewMatrix[3].x, m_InverseViewMatrix[3].y, m_InverseViewMatrix[3].z);
}

glm::vec3 CCamera::GetLookAt()
{
	return -glm::vec3(m_InverseViewMatrix[2].x, m_InverseViewMatrix[2].y, m_InverseViewMatrix[2].z);
}


void CCamera::SetPosition(glm::vec3 translate)
{
	m_InverseViewMatrix = glm::inverse(m_ViewMatrix);
	m_InverseViewMatrix[3].x = translate.x;
	m_InverseViewMatrix[3].y = translate.y;
	m_InverseViewMatrix[3].z = translate.z;
	m_ViewMatrix = glm::inverse(m_InverseViewMatrix);
}

glm::vec3 CCamera::GetCameraAxisInWorldSpace(int axis)
{

	//get x axis in world space
	if (axis == 0)
		return glm::vec3(m_InverseViewMatrix[0].x, m_InverseViewMatrix[0].y, m_InverseViewMatrix[0].z);

	//TODO
}

void CCamera::RotateCamera(float angle, const glm::vec3& axis)
{
	m_ViewMatrix = glm::rotate(m_ViewMatrix, glm::radians(angle), axis);
	m_InverseViewMatrix = glm::inverse(m_ViewMatrix);
}

void CCamera::TranslateCamera(const glm::vec3& t)
{
//	m_InverseViewMatrix = glm::inverse(m_ViewMatrix);
	m_InverseViewMatrix[3].x += t.x;
	m_InverseViewMatrix[3].y += t.y;
	m_InverseViewMatrix[3].z += t.z;
	m_ViewMatrix = glm::inverse(m_InverseViewMatrix);
}
