#pragma once

#include "ModelPositionData.h"
#include "ModelColorData.h"
#include "glm\gtc\matrix_transform.hpp"

#include <vector>


class CRoundedSquarePositionData : public CModelPositionData
{
public:

	CRoundedSquarePositionData(float size) : m_Size(size)
	{}

	void GeneratePositionBuffer() override;
	void GenerateVertices(std::vector<float>& vertices);

private:

	float m_Size;
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