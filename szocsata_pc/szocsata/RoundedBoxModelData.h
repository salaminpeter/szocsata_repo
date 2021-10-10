#pragma once

#include "ModelPositionData.h"
#include "ModelColorData.h"
#include "glm\gtc\matrix_transform.hpp"

#include <vector>

class CRoundedBoxPositionData : public CModelPositionData
{
public:
	
	CRoundedBoxPositionData(int sideLod, float sideRadius, int edgeLod, float edgeRadius) :
		m_SideLOD(sideLod),
		m_SideRadius(sideRadius),
		m_EdgeLod(edgeLod),
		m_EdgeRadius(edgeRadius)
	{}

	void GeneratePositionBuffer() override;
	void GenerateRoundedBoxVertices();

	std::vector<glm::vec3>& GetTopVertices() { return m_TopVertices; }

private:

	int m_SideLOD;
	int m_EdgeLod;
	float m_SideRadius;
	float m_EdgeRadius;

	static std::vector<glm::vec3> m_TopVertices;
};


class CRoundedBoxColorData : public CModelColorData
{
public:

	CRoundedBoxColorData(float topTextLeft, float topTextTop, float topTextRight, float topTextBottom, float sideTextLeft, float sideTextTop, float sideTextRight, float sideTextBottom, unsigned gridX, unsigned gridY) :
		m_TopTextureLeft(topTextLeft),
		m_TopTextureTop(topTextTop),
		m_TopTextureRight(topTextRight),
		m_TopTextureBottom(topTextBottom),
		m_SideTextureLeft(sideTextLeft),
		m_SideTextureTop(sideTextTop),
		m_SideTextureRight(sideTextRight),
		m_SideTextureBottom(sideTextBottom),
		m_GridXSize(gridX),
		m_GridYSize(gridY)
	{} //TODO kell ez a konstruktor??

	CRoundedBoxColorData() {}

	void SetTexturePositions(float topTextLeft, float topTextTop, float topTextRight, float topTextBottom, float sideTextLeft, float sideTextTop, float sideTextRight, float sideTextBottom);
	void GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions) override;

	unsigned m_Offset = 0;

private:

	float m_TopTextureLeft;
	float m_TopTextureTop;
	float m_TopTextureRight;
	float m_TopTextureBottom;
	float m_SideTextureLeft;
	float m_SideTextureTop;
	float m_SideTextureRight;
	float m_SideTextureBottom;
	unsigned m_GridXSize;
	unsigned m_GridYSize;
};
