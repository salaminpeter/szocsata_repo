#pragma once

#include "ModelColorData.h"
#include "ModelPositionData.h"

class CBoardPositionData : public CModelPositionData
{
public:

	CBoardPositionData(int lod) : m_LOD(lod) {}

	void GenerateBoardVertices();
	void GeneratePositionBuffer() override;

	std::vector<glm::vec3>& GetVertices() { return m_Vertices; }

private:

	int m_LOD;
	std::vector<glm::vec3> m_Vertices;
	std::vector<glm::vec3> m_Normals;
};

class CBoardColorData : public CModelColorData
{
public:
	
	CBoardColorData(int lod) : m_LOD(lod) {}

	void GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions) override;

private:

	int m_LOD;
};