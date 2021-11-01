#include "stdafx.h"
#include "SquareModelData.h"
#include "Config.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

void CRoundedSquarePositionData::GenerateVertices(std::vector<float>& vertices)
{
	int SideLOD;
	float SideRadius;

	CConfig::GetConfig("letter_side_lod", SideLOD);
	CConfig::GetConfig("letter_side_radius", SideRadius);

	//TODO vertices reserve!!
	glm::vec3 ArchOrigo(-0.5f * m_Size + SideRadius, -0.5f * m_Size + SideRadius, 0.f);
	glm::vec3 ArchRadiusVec = glm::vec3(-SideRadius, 0.f, 0.f);

	float Angle = glm::radians(90.f / (SideLOD + 1));

	//top left corner
	for (int i = 0; i < SideLOD + 2; ++i)
	{
		glm::vec3 VertexPos = ArchRadiusVec + ArchOrigo;
		vertices.push_back(VertexPos.x);
		vertices.push_back(VertexPos.y);
		vertices.push_back(VertexPos.z);

		float NewX = ArchRadiusVec.x * std::cosf(Angle) - ArchRadiusVec.y * std::sinf(Angle);
		float NewY = ArchRadiusVec.x * std::sinf(Angle) + ArchRadiusVec.y * std::cosf(Angle);

		ArchRadiusVec.x = NewX;
		ArchRadiusVec.y = NewY;
	}

	
	//top right corner
	int From = vertices.size() - 1;
	for (int i = From; i >= 2; i -= 3)
	{
		glm::vec3 VertexPos = glm::vec3(vertices[i - 2], vertices[i - 1], vertices[i]) * glm::vec3(-1.f, 1.f, 0.f);
		vertices.push_back(VertexPos.x);
		vertices.push_back(VertexPos.y);
		vertices.push_back(VertexPos.z);
	}
	
	//bottom right corner
	From = vertices.size() - 1;
	int To = From - SideLOD * 3;
	for (int i = From; i >= To; i -= 3)
	{
		glm::vec3 VertexPos = glm::vec3(vertices[i - 2], vertices[i - 1], vertices[i]) * glm::vec3(1.f, -1.f, 0.f);
		vertices.push_back(VertexPos.x);
		vertices.push_back(VertexPos.y);
		vertices.push_back(VertexPos.z);
	}
	
	//bottom left corner
	From = vertices.size() - 1;
	To = From - SideLOD * 3 - 3;
	for (int i = From; i >= To; i -= 3)
	{
		glm::vec3 VertexPos = glm::vec3(vertices[i - 2], vertices[i - 1], vertices[i]) * glm::vec3(-1.f, 1.f, 0.f);
		vertices.push_back(VertexPos.x);
		vertices.push_back(VertexPos.y);
		vertices.push_back(VertexPos.z);
	}
	
	//origo
	vertices.insert(vertices.begin(), 0.f);
	vertices.insert(vertices.begin(), 0.f);
	vertices.insert(vertices.begin(), 0.f);
}

void CRoundedSquarePositionData::GeneratePositionBuffer()
{
	std::vector<float> Vertices;
	GenerateVertices(Vertices);

	std::vector<unsigned int> Indices; //TODO reserve

	//top indices
	for (size_t i = 0; i < Vertices.size(); ++i)
	{
		Indices.push_back(i + 1);
		Indices.push_back(i);
		Indices.push_back(0);
	}

	
	Indices.push_back(1);
	Indices.push_back(Vertices.size());
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
