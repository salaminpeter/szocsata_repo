#include "stdafx.h"
#include "AnimationPath.h"

void CAnimationPath::CreatePath(const std::vector<glm::vec2>& controlPoints, int resolution)
{
	m_Path.reserve(1000); //TODO ertelmes ertek splinhossz fuggvenyeben
	m_Path.clear();

	int SectionCount = controlPoints.size() - 3;

	for (int i = 0; i < SectionCount; ++i)
	{
		const glm::vec2& p0 = controlPoints[i];
		const glm::vec2& p1 = controlPoints[i + 1];
		const glm::vec2& p2 = controlPoints[i + 2];
		const glm::vec2& p3 = controlPoints[i + 3];

		float t01 = glm::pow(distance(p0, p1), m_Alpha);
		float t12 = glm::pow(distance(p1, p2), m_Alpha);
		float t23 = glm::pow(distance(p2, p3), m_Alpha);

		glm::vec2 m1 = (1.0f - m_Tension) * (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
		glm::vec2 m2 = (1.0f - m_Tension) * (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));

		glm::vec2 SegVecA = 2.f * (p1 - p2) + m1 + m2;
		glm::vec2 SegVecB = -3.0f * (p1 - p2) - m1 - m1 - m2;
		glm::vec2 SegVecC = m1;
		glm::vec2 SegVecD = p1;

		int n = distance(p1, p2) * 1.5f;

		for (int j = 0; j < n; ++j)
		{
			float t = static_cast<float>(j) / static_cast<float>(n);
			glm::vec2 Point = SegVecA * t * t * t + SegVecB * t * t + SegVecC * t + SegVecD;

			if (m_Path.size() == 0 || glm::distance(m_Path.back(), Point) > static_cast<float>(resolution))
			{
				m_Path.emplace_back(Point.x, Point.y);
			}
		}
	}
}

glm::vec2 CAnimationPath::GetPathPoint(float t)
{
	if (t < 0.f || t >= 1.f)
		return glm::vec2(-1, -1);

	size_t Idx = static_cast<size_t>(t * (static_cast<float>(m_Path.size() - 1)));
	Idx = Idx >= m_Path.size() ? m_Path.size() - 1 : Idx;

	return m_Path[Idx];
}
