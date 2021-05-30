#pragma once

#include "ModelPositionData.h"
#include "ModelColorData.h"
#include "glm\gtc\matrix_transform.hpp"

#include <vector>


class CRoundedSquarePositionData : public CModelPositionData
{
public:

	CRoundedSquarePositionData(int lod, float radius) :
		m_LOD(lod),
		m_Radius(radius)
	{}

	void GeneratePositionBuffer() override;
	void GenerateRoundedBoxVertices();

	std::vector<glm::vec3>& GetVertices() { return m_Vertices; }
	std::vector<glm::vec2>& GetNormals() { return m_SideNormals; }

private:

	int m_LOD;
	float m_Radius;

	std::vector<glm::vec3> m_Vertices;
	std::vector<glm::vec2> m_SideNormals;
};


class CSquarePositionData : public CModelPositionData
{
public:

	void GeneratePositionBuffer() override;
};

class CSquareColorData : public CModelColorData
{
public:
	
	float m_DivX = 1.f;
	float m_DivY = 1.f;

	void GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions) override;
};