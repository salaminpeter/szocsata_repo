#include "stdafx.h"
#include "BoardModelData.h"
#include "Config.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include <vector>


void CBoardPositionData::GenerateBoardVertices()
{
	//TODO reserve m_Vertices m_Normals

	float TileHeight;
	float BoardSize;
	float BoardHeight;

	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("board_height", BoardHeight);

	BoardHeight /= 2.f;
	
	//top corners
	m_Vertices.push_back(glm::vec3(-BoardSize, -BoardSize, BoardHeight));
	m_Vertices.push_back(glm::vec3(-BoardSize, BoardSize, BoardHeight));
	m_Vertices.push_back(glm::vec3(BoardSize, BoardSize, BoardHeight));
	m_Vertices.push_back(glm::vec3(BoardSize, -BoardSize, BoardHeight));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));
	m_Normals.push_back(glm::vec3(0.f, 0.f, 1.f));

	//side vertices
	glm::vec2 RadiusVec(0.f, BoardHeight * 2.f);

	for (int i = 0; i < m_LOD + 1; ++i)
	{
		float Angle = glm::radians(90.f / (m_LOD + 1));

		float NewX = RadiusVec.x * std::cosf(-Angle) - RadiusVec.y * std::sinf(-Angle);
		float NewY = RadiusVec.x * std::sinf(-Angle) + RadiusVec.y * std::cosf(-Angle);

		RadiusVec.x = NewX;
		RadiusVec.y = NewY;

		float VertexX = RadiusVec.x / 1.41;
		float VertexY = VertexX;
		float VertexZ = NewY;
		glm::vec3 Normal;

		//bottom left
		m_Vertices.push_back(glm::vec3(-BoardSize - VertexX, -BoardSize - VertexY, -BoardHeight + VertexZ));
		Normal = glm::normalize(glm::vec3(-RadiusVec.x, 0.f, RadiusVec.y));
		m_Normals.push_back(Normal);
		Normal = glm::normalize(glm::vec3(0.f, -RadiusVec.x, RadiusVec.y));
		m_Normals.push_back(Normal);

		//top left
		m_Vertices.push_back(glm::vec3(-BoardSize - VertexX, BoardSize + VertexY, -BoardHeight + VertexZ));
		Normal = glm::normalize(glm::vec3(-RadiusVec.x, 0.f, RadiusVec.y));
		m_Normals.push_back(Normal);
		Normal = glm::normalize(glm::vec3(0.f, RadiusVec.x, RadiusVec.y));
		m_Normals.push_back(Normal);

		//top right
		m_Vertices.push_back(glm::vec3(BoardSize + VertexX, BoardSize + VertexY, -BoardHeight + VertexZ));
		Normal = glm::normalize(glm::vec3(RadiusVec.x, 0.f, RadiusVec.y));
		m_Normals.push_back(Normal);
		Normal = glm::normalize(glm::vec3(0.f, RadiusVec.x, RadiusVec.y));
		m_Normals.push_back(Normal);

		//bottom right
		m_Vertices.push_back(glm::vec3(BoardSize + VertexX, -BoardSize - VertexY, -BoardHeight + VertexZ));
		Normal = glm::normalize(glm::vec3(RadiusVec.x, 0.f, RadiusVec.y));
		m_Normals.push_back(Normal);
		Normal = glm::normalize(glm::vec3(0.f, -RadiusVec.x, RadiusVec.y));
		m_Normals.push_back(Normal);
	}

}

void CBoardPositionData::GeneratePositionBuffer()
{
	GenerateBoardVertices();

	std::vector<float> VertexAttribs;

	//TODO vertexattribs reserve
	
	//top
	for (int j = 0; j < 4; ++j)
	{
		VertexAttribs.push_back(m_Vertices[j].x);
		VertexAttribs.push_back(m_Vertices[j].y);
		VertexAttribs.push_back(m_Vertices[j].z);
		VertexAttribs.push_back(0.f);
		VertexAttribs.push_back(0.f);
		VertexAttribs.push_back(1.f);
	}

	//sides
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < m_LOD + 2; ++i)
		{
			size_t vidx0 = i * 4 + j;
			size_t vidx1 = i * 4 + (j != 3 ? j + 1 : 0);

			size_t nidx0 = i * 4 * 2 + (j % 2) + (j * 2);
			size_t nidx1 = i * 4 * 2 + (j % 2) + (j != 3 ? j * 2 + 2 : 0);

			VertexAttribs.push_back(m_Vertices[vidx0].x);
			VertexAttribs.push_back(m_Vertices[vidx0].y);
			VertexAttribs.push_back(m_Vertices[vidx0].z);
			VertexAttribs.push_back(m_Normals[nidx0].x);
			VertexAttribs.push_back(m_Normals[nidx0].y);
			VertexAttribs.push_back(m_Normals[nidx0].z);

			VertexAttribs.push_back(m_Vertices[vidx1].x);
			VertexAttribs.push_back(m_Vertices[vidx1].y);
			VertexAttribs.push_back(m_Vertices[vidx1].z);
			VertexAttribs.push_back(m_Normals[nidx1].x);
			VertexAttribs.push_back(m_Normals[nidx1].y);
			VertexAttribs.push_back(m_Normals[nidx1].z);
		}
	}
	
	std::vector<unsigned int> Indices;
	
	//top indices
	Indices.push_back(0);
	Indices.push_back(1);
	Indices.push_back(3);
	Indices.push_back(1);
	Indices.push_back(2);
	Indices.push_back(3);

	
	//side indices
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < m_LOD + 1; ++i)
		{
			unsigned idx = ((m_LOD + 2) * 2 * j) + i * 2 + 4;
			
			Indices.push_back(idx);
			Indices.push_back(idx + 2);
			Indices.push_back(idx + 1);
			Indices.push_back(idx + 2);
			Indices.push_back(idx + 3);
			Indices.push_back(idx + 1);
		}
	}

	m_IndexCount = Indices.size();

	glGenBuffers(1, (GLuint*)&m_IndexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, (GLuint*)&m_VertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, VertexAttribs.size() * sizeof(GLfloat), &VertexAttribs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void CBoardColorData::GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions)
{
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);

	float CornerSectionLength = glm::length(glm::vec3(vertexPositions[4].x - vertexPositions[0].x, vertexPositions[4].y - vertexPositions[0].y, vertexPositions[4].z - vertexPositions[0].z));
	float CornerLength = (m_LOD + 1) * CornerSectionLength;

	float SideLength = BoardSize + (CornerLength / 1.41f) * 2;

	//TODO reserve texcoordbuffer
	std::vector<float> TexCoordBuffer;

	//top side
	float d = CornerLength / 1.41;

	//bottom left
	TexCoordBuffer.push_back(d / SideLength);
	TexCoordBuffer.push_back(d / SideLength);

	//top left
	TexCoordBuffer.push_back(d / SideLength);
	TexCoordBuffer.push_back(1.f - d / SideLength);

	//top right
	TexCoordBuffer.push_back(1.f - d / SideLength);
	TexCoordBuffer.push_back(1.f - d / SideLength);

	//bottom right
	TexCoordBuffer.push_back(1.f - d / SideLength);
	TexCoordBuffer.push_back(d / SideLength);

	//left
	float Dist = 0.f;

	for (int i = 0; i < m_LOD + 2; ++i)
	{
		float d = (Dist / 1.41) / SideLength;

		TexCoordBuffer.push_back(TexCoordBuffer[0] - d);
		TexCoordBuffer.push_back(TexCoordBuffer[1] - d);
		TexCoordBuffer.push_back(TexCoordBuffer[2] - d);
		TexCoordBuffer.push_back(TexCoordBuffer[3] + d);

		Dist += CornerSectionLength;
	}

	//top
	Dist = 0.f;
	
	for (int i = 0; i < m_LOD + 2; ++i)
	{
		float d = (Dist / 1.41) / SideLength;

		TexCoordBuffer.push_back(TexCoordBuffer[2] - d);
		TexCoordBuffer.push_back(TexCoordBuffer[3] + d);
		TexCoordBuffer.push_back(TexCoordBuffer[4] + d);
		TexCoordBuffer.push_back(TexCoordBuffer[5] + d);

		Dist += CornerSectionLength;
	}

	//right
	Dist = 0.f;

	for (int i = 0; i < m_LOD + 2; ++i)
	{
		float d = (Dist / 1.41) / SideLength;

		TexCoordBuffer.push_back(TexCoordBuffer[4] + d);
		TexCoordBuffer.push_back(TexCoordBuffer[5] + d);
		TexCoordBuffer.push_back(TexCoordBuffer[6] + d);
		TexCoordBuffer.push_back(TexCoordBuffer[7] - d);

		Dist += CornerSectionLength;
	}

	//bottom
	Dist = 0.f;

	for (int i = 0; i < m_LOD + 2; ++i)
	{
		float d = (Dist / 1.41) / SideLength;

		TexCoordBuffer.push_back(TexCoordBuffer[6] + d);
		TexCoordBuffer.push_back(TexCoordBuffer[7] - d);
		TexCoordBuffer.push_back(TexCoordBuffer[0] - d);
		TexCoordBuffer.push_back(TexCoordBuffer[1] - d);

		Dist += CornerSectionLength;
	}

	glGenBuffers(1, (GLuint*)&m_TextureCoordBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, TexCoordBuffer.size() * sizeof(GLfloat), &TexCoordBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
