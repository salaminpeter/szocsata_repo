#include "stdafx.h"
#include "SquareModelData.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

void CRoundedSquarePositionData::GenerateRoundedBoxVertices()
{
	m_Vertices.clear();

	float size = 1.f;

	m_Vertices.reserve((m_LOD + 2) * 4);

	glm::vec3 ArchOrigo(-0.5f * size + m_Radius, -0.5f * size + m_Radius, 0.f);
	glm::vec3 ArchRadiusVec = glm::vec3(-m_Radius, 0.f, 0.f);

	float Angle = glm::radians(90.f / (m_LOD + 1));

	//top left corner
	for (int i = 0; i < m_LOD + 2; ++i)
	{
		glm::vec3 VertexPos = ArchRadiusVec + ArchOrigo;
		m_Vertices.push_back(VertexPos);
		m_SideNormals.push_back(glm::normalize(ArchRadiusVec));

		float NewX = ArchRadiusVec.x * std::cosf(Angle) - ArchRadiusVec.y * std::sinf(Angle);
		float NewY = ArchRadiusVec.x * std::sinf(Angle) + ArchRadiusVec.y * std::cosf(Angle);

		ArchRadiusVec.x = NewX;
		ArchRadiusVec.y = NewY;
	}


	//top right corner
	for (int i = m_LOD + 1; i >= 0; --i)
	{
		glm::vec3 VertexPos = m_Vertices[i] * glm::vec3(-1.f, 1.f, 0.f);
		m_Vertices.push_back(VertexPos);
		glm::vec2 VertexNormal = m_SideNormals[i] * glm::vec2(-1.f, 1.f);
		m_SideNormals.push_back(VertexNormal);
	}

	//bottom right corner
	int From = m_Vertices.size() - 1;
	for (int i = From; i >= From - m_LOD - 1; --i)
	{
		glm::vec3 VertexPos = m_Vertices[i] * glm::vec3(1.f, -1.f, 0.f);
		m_Vertices.push_back(VertexPos);
		glm::vec2 VertexNormal = m_SideNormals[i] * glm::vec2(1.f, -1.f);
		m_SideNormals.push_back(VertexNormal);
	}

	//bottom left corner
	From = m_Vertices.size() - 1;
	for (int i = From; i >= From - m_LOD - 1; --i)
	{
		glm::vec3 VertexPos = m_Vertices[i] * glm::vec3(-1.f, 1.f, 0.f);
		m_Vertices.push_back(VertexPos);
		glm::vec2 VertexNormal = m_SideNormals[i] * glm::vec2(-1.f, 1.f);
		m_SideNormals.push_back(VertexNormal);
	}
}

void CRoundedSquarePositionData::GeneratePositionBuffer()
{
	std::vector<float> Vertices;
	//TODO reserve

	//origo
	Vertices.push_back(0.f);
	Vertices.push_back(0.f);
	Vertices.push_back(0.f);

	//top vertex attributes
	for (size_t i = 0; i < m_Vertices.size(); ++i)
	{
		Vertices.push_back(m_Vertices[i].x);
		Vertices.push_back(m_Vertices[i].y);
		Vertices.push_back(0.f);
	}

	std::vector<unsigned int> Indices; //TODO reserve

	//top indices
	for (size_t i = 0; i < m_Vertices.size(); ++i)
	{
		Indices.push_back(i + 1);
		Indices.push_back(i);
		Indices.push_back(0);
	}

	
	Indices.push_back(1);
	Indices.push_back(m_Vertices.size());
	Indices.push_back(0);
	
	m_IndexCount = Indices.size();

	glGenBuffers(1, (GLuint*)&m_IndexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, (GLuint*)&m_VertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(GLfloat), &Vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void CSquarePositionData::GeneratePositionBuffer()
{
	std::vector<float> Vertices;
	Vertices.reserve(12);

	Vertices.push_back(-0.5f);
	Vertices.push_back(-0.5f);
	Vertices.push_back(0.f);

	Vertices.push_back(-0.5f);
	Vertices.push_back(0.5f);
	Vertices.push_back(0.f);

	Vertices.push_back(0.5f);
	Vertices.push_back(0.5f);
	Vertices.push_back(0.f);

	Vertices.push_back(0.5f);
	Vertices.push_back(-0.5f);
	Vertices.push_back(0.f);

	std::vector<unsigned int> Indices;
	Indices.reserve(6);

	Indices.push_back(0);
	Indices.push_back(1);
	Indices.push_back(2);

	Indices.push_back(0);
	Indices.push_back(2);
	Indices.push_back(3);

	m_IndexCount = Indices.size();

	glGenBuffers(1, (GLuint*)&m_IndexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, (GLuint*)&m_VertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(GLfloat), &Vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CSquareColorData::GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions)
{
	std::vector<float> TexCoordBuffer;
	TexCoordBuffer.reserve(8);

	TexCoordBuffer.push_back(0.f);
	TexCoordBuffer.push_back(0.f);

	TexCoordBuffer.push_back(0.f);
	TexCoordBuffer.push_back(1.f / m_DivY);

	TexCoordBuffer.push_back(1.f / m_DivX);
	TexCoordBuffer.push_back(1.f / m_DivY);

	TexCoordBuffer.push_back(1.f / m_DivX);
	TexCoordBuffer.push_back(0.f);
	
	glGenBuffers(1, (GLuint*)&m_TextureCoordBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, TexCoordBuffer.size() * sizeof(GLfloat), &TexCoordBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
