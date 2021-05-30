
#pragma once

#include "glm\gtc\matrix_transform.hpp"
#include <glm/gtx/rotate_vector.hpp>

#include <string>

class CCamera
{
public:


	void InitPerspective(float fovY, float ratio, float nearZ, float farZ)
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(fovY), ratio, nearZ, farZ);
	}

	void InitOrtho(float left, float right, float top, float bottom)
	{
		//m_ProjectionMatrix = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.0f, 100.0f);
		//m_ProjectionMatrix = glm::ortho(-500.f, 500.0f, -500.f, 500.f, 0.0f, 100.0f);
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, 0.0f, 100.0f);
	}


	void InitView(const glm::vec3& position, const glm::vec3& lookAtPosition, const glm::vec3& upVector)
	{
		m_ViewMatrix = glm::lookAt(position, lookAtPosition, upVector);
		m_LookVector = glm::normalize(lookAtPosition - position);
		m_UpVector = upVector;
		m_Position = position;
		m_NeedLookAt = true;
	}

	void Move(float distance)
	{
		m_Position[0] += m_LookVector[0] * distance;
		m_Position[1] += m_LookVector[1] * distance;
		m_Position[2] += m_LookVector[2] * distance;
		m_NeedLookAt = true;
	}

	void SetPosition(glm::vec3 translate)
	{
		glm::mat4 InverseView = glm::inverse(m_ViewMatrix);
		InverseView[3].x = translate.x;
		InverseView[3].y = translate.y;
		InverseView[3].z = translate.z;
		m_Position[0] = InverseView[3].x;
		m_Position[1] = InverseView[3].y;
		m_Position[2] = InverseView[3].z;
		m_ViewMatrix = glm::inverse(InverseView);
		m_NeedLookAt = true;
	}

	void Rotate(float angle, glm::vec3 axis, bool axisInWorldSpace = false)
	{
		if (axisInWorldSpace)
			axis = m_ViewMatrix * glm::vec4(axis, 0);
		
		glm::mat4 RotMatrix = glm::mat4(1.0f);
		RotMatrix = glm::rotate(RotMatrix, glm::radians(angle), axis);
		m_ViewMatrix = m_ViewMatrix * RotMatrix;

		glm::mat4 InverseView = glm::inverse(m_ViewMatrix); //TODO inversematrix is tarolva legyen memberkent

		m_LookVector[0] = -InverseView[2].x;
		m_LookVector[1] = -InverseView[2].y;
		m_LookVector[2] = -InverseView[2].z;

		m_Position[0] = InverseView[3].x;
		m_Position[1] = InverseView[3].y;
		m_Position[2] = InverseView[3].z;
		m_NeedLookAt = true;
	}

	glm::vec3 GetCameraAxisInWorldSpace(int axis)
	{
		glm::mat4 InverseView = glm::inverse(m_ViewMatrix); //TODO inversematrix is tarolva legyen memberkent
															
		//get x axis in world space
		if (axis == 0)
			return glm::vec3(InverseView[0].x, InverseView[0].y, InverseView[0].z);

		//TODO
	}

	void LookAt(bool force = false)
	{
		if (m_NeedLookAt || force)
		{
			m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_LookVector, m_UpVector);
			m_NeedLookAt = false;
		}
	}

	void SetPosition(float x, float y, float z)
	{
		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
		m_NeedLookAt = true;
	}

public:

	const glm::mat4& ProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

	const glm::mat4& ViewMatrix() const
	{
		return m_ViewMatrix;
	}

	const glm::vec3& LookAtVector() const
	{
		return m_LookVector;
	}

public:

	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::vec3 m_LookVector;
	glm::vec3 m_UpVector;
	glm::vec3 m_Position;

	bool m_NeedLookAt = true;
};