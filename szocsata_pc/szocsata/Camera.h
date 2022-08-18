#pragma once

#include <glm/glm.hpp>

class CCamera
{
public:

	void InitPerspective(float fovY, float ratio, float nearZ, float farZ);
	void InitOrtho(float left, float right, float top, float bottom);
	void InitView(const glm::vec3& position, const glm::vec3& lookAtPosition, const glm::vec3& upVector);
	void SetPosition(glm::vec3 translate);
	glm::vec3 GetLookAt();
	glm::vec3 GetPosition();
	glm::vec3 GetCameraAxisInWorldSpace(int axis);

	void RotateCamera(float angle, const glm::vec3& axis);
	void TranslateCamera(const glm::vec3& t);

	const glm::mat4& ProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

	const glm::mat4& ViewMatrix() const
	{
		return m_ViewMatrix;
	}

private:

	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_InverseViewMatrix;
};