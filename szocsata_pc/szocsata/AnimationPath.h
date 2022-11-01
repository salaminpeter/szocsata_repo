#pragma once

#include <vector>
#include <glm/glm.hpp>

class CAnimationPath
{
public:

	void CreatePath(const std::vector<glm::vec2>& controlPoints, int resolution);
	glm::vec2 GetPathPoint(float t);

private:

	const float m_Alpha = .5f;
	const float m_Tension = .0f;

	std::vector<glm::vec2> m_Path;
};
