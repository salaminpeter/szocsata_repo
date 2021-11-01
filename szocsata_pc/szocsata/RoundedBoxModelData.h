#pragma once

#include "ModelPositionData.h"
#include "ModelColorData.h"
#include "glm\gtc\matrix_transform.hpp"

#include <vector>

class CRoundedBoxPositionData : public CModelPositionData
{
public:
	
	CRoundedBoxPositionData(float size, float height, float edgeRadius) :
		m_Size(size),
		m_Height(height),
		m_EdgeRadius(edgeRadius)
	{}

	void GeneratePositionBuffer() override;
	void GenerateRoundedBoxVertices();

	std::vector<glm::vec3> GetTopVertices(bool inner);

private:

	float m_Size;
	float m_Height;
	float m_EdgeRadius;

	static std::vector<glm::vec2> m_TopVerticesInner;
	static std::vector<glm::vec2> m_TopVerticesOuter;
};

class CRoundedBoxColorData : public CModelColorData
{
public:

	CRoundedBoxColorData(float topTextLeft, float topTextTop, float topTextRight, float topTextBottom, float sideTextLeft, float sideTextTop, float sideTextRight, float sideTextBottom, unsigned gridX, unsigned gridY, float edgeRadius) :
		m_TopTextureLeft(topTextLeft),
		m_TopTextureTop(topTextTop),
		m_TopTextureRight(topTextRight),
		m_TopTextureBottom(topTextBottom),
		m_SideTextureLeft(sideTextLeft),
		m_SideTextureTop(sideTextTop),
		m_SideTextureRight(sideTextRight),
		m_SideTextureBottom(sideTextBottom),
		m_GridXSize(gridX),
		m_GridYSize(gridY),
		m_EdgeRadius(edgeRadius)
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
	float m_EdgeRadius;
};
