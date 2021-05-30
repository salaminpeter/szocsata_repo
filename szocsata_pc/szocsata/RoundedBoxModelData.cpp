#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "RoundedBoxModelData.h"
#include "Config.h"

#include <vector>

//TODO a RoundedSquare-t hasznalja ez is!
std::vector<glm::vec3> CRoundedBoxPositionData::m_TopVertices;


void CRoundedBoxPositionData::GenerateRoundedBoxVertices(std::vector<glm::vec2>* sideNormals)
{
	if (m_TopVertices.size())
		return;

	float size = 1.f;

	m_TopVertices.reserve((m_LOD + 2) * 4);

	glm::vec3 ArchOrigo(-0.5f * size + m_Radius, -0.5f * size + m_Radius, 0.f);
	glm::vec3 ArchRadiusVec = glm::vec3(-m_Radius, 0.f, 0.f);

	float Angle = glm::radians(90.f / (m_LOD + 1));

	//top left corner
	for (int i = 0; i < m_LOD + 2; ++i)
	{
		glm::vec3 VertexPos = ArchRadiusVec + ArchOrigo;
		m_TopVertices.push_back(VertexPos);

		if (sideNormals)
			sideNormals->push_back(glm::normalize(ArchRadiusVec));

		float NewX = ArchRadiusVec.x * std::cosf(Angle) - ArchRadiusVec.y * std::sinf(Angle);
		float NewY = ArchRadiusVec.x * std::sinf(Angle) + ArchRadiusVec.y * std::cosf(Angle);

		ArchRadiusVec.x = NewX;
		ArchRadiusVec.y = NewY;
	}
	
	//top right corner
	for (int i = m_LOD + 1; i >= 0; --i)
	{
		glm::vec3 VertexPos = m_TopVertices[i] * glm::vec3(-1.f, 1.f, 0.f);
		m_TopVertices.push_back(VertexPos);

		if (sideNormals)
		{ 
			glm::vec2 VertexNormal = (*sideNormals)[i] * glm::vec2(-1.f, 1.f);
			sideNormals->push_back(VertexNormal);
		}
	}
	
	//bottom right corner
	int From = m_TopVertices.size() - 1;
	for (int i = From; i >= From - m_LOD - 1; --i)
	{
		glm::vec3 VertexPos = m_TopVertices[i] * glm::vec3(1.f, -1.f, 0.f);
		m_TopVertices.push_back(VertexPos);

		if (sideNormals)
		{
			glm::vec2 VertexNormal = (*sideNormals)[i] * glm::vec2(1.f, -1.f);
			sideNormals->push_back(VertexNormal);
		}
	}
	
	//bottom left corner
	From = m_TopVertices.size() - 1;
	for (int i = From; i >= From - m_LOD - 1; --i)
	{
		glm::vec3 VertexPos = m_TopVertices[i] * glm::vec3(-1.f, 1.f, 0.f);
		m_TopVertices.push_back(VertexPos);

		if (sideNormals)
		{
			glm::vec2 VertexNormal = (*sideNormals)[i] * glm::vec2(-1.f, 1.f);
			sideNormals->push_back(VertexNormal);
		}
	}
}

void CRoundedBoxPositionData::GeneratePositionBuffer()
{
	std::vector<glm::vec2> SideNormals;

	GenerateRoundedBoxVertices(&SideNormals);

	std::vector<float> Vertices;
	//TODO reserve
	
	//origo
	Vertices.push_back(0.f);
	Vertices.push_back(0.f);
	Vertices.push_back(0.5f);

	Vertices.push_back(0.f);
	Vertices.push_back(0.f);
	Vertices.push_back(1.f);

	//top vertex attributes
	for (size_t i = 0; i < m_TopVertices.size(); ++i)
	{
		Vertices.push_back(m_TopVertices[i].x);
		Vertices.push_back(m_TopVertices[i].y);
		Vertices.push_back(0.5f);

		Vertices.push_back(0.f);
		Vertices.push_back(0.f);
		Vertices.push_back(1.f);
	}

	//side vertex attributes
	for (size_t i = 0; i <= m_TopVertices.size(); ++i)
	{
		float SideTexX;
		int idx = i < m_TopVertices.size() ? i : 0;

		for (int j = 0; j < 2; ++j)
		{
			Vertices.push_back(m_TopVertices[idx].x);
			Vertices.push_back(m_TopVertices[idx].y);
			Vertices.push_back(j == 0 ? 0.5f : -0.5f);

			Vertices.push_back(SideNormals[idx].x);
			Vertices.push_back(SideNormals[idx].y);
			Vertices.push_back(0.f);
		}
	}
	
	std::vector<unsigned int> Indices;

	//top indices
	for (size_t i = 0; i < m_TopVertices.size(); ++i)
	{
		Indices.push_back(i + 1);
		Indices.push_back(i);
		Indices.push_back(0);
	}
	
	Indices.push_back(1);
	Indices.push_back(m_TopVertices.size());
	Indices.push_back(0);
	
	//side indices
	int Offset = m_TopVertices.size() + 1;

	for (size_t i = 0; i < (m_TopVertices.size() - 1) * 2 + 2; ++i)
	{
		Indices.push_back((i % 2) ? Offset + i : Offset + i + 2);
		Indices.push_back(Offset + i + 1);
		Indices.push_back((i % 2) ? Offset + i + 2 : Offset + i);
	}

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


void CRoundedBoxColorData::SetTexturePositions(float topTextLeft, float topTextTop, float topTextRight, float topTextBottom, float sideTextLeft, float sideTextTop, float sideTextRight, float sideTextBottom)
{
	m_TopTextureLeft = topTextLeft;
	m_TopTextureTop = topTextTop;
	m_TopTextureRight = topTextRight;
	m_TopTextureBottom = topTextBottom;
	m_SideTextureLeft = sideTextLeft;
	m_SideTextureTop = sideTextTop;
	m_SideTextureRight = sideTextRight;
	m_SideTextureBottom = sideTextBottom;
}


void CRoundedBoxColorData::GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);


	//TODO a tile texturanal az y - ok tukrozve vannak - javitani (meg kell nezni egy szabalyos texturaval hogy latszodjon a hiba)
	//TODO reserve texcoordbuffer
	std::vector<float> TexCoordBuffer;

	float TextWidth = m_TopTextureRight - m_TopTextureLeft;
	float TextHeight = m_TopTextureTop - m_TopTextureBottom;

	for (unsigned y = 0; y < m_GridYSize; ++y) 
	{
		for (unsigned x = 0; x < m_GridXSize; ++x)
		{
			//tex coordinates for midle of top side
			TexCoordBuffer.push_back(x * TextWidth + m_TopTextureLeft + TextWidth / 2.f);
			TexCoordBuffer.push_back(y * TextHeight + m_TopTextureBottom + TextHeight / 2.f);

			//tex coordinates for top
			for (size_t i = 0; i < vertexPositions.size(); ++i)
			{
				TexCoordBuffer.push_back(x * TextWidth + m_TopTextureLeft + TextWidth * (vertexPositions[i].x + .5f));
				TexCoordBuffer.push_back(y * TextHeight + m_TopTextureBottom + TextHeight * (vertexPositions[i].y + .5f));
			}

			//tex coordinates for side
			float CurrLength = 0;
			float SideLength = 0;

			for (size_t i = 0; i < vertexPositions.size() / 4; ++i)
				SideLength += std::sqrtf((vertexPositions[i].x - vertexPositions[i + 1].x) * (vertexPositions[i].x - vertexPositions[i + 1].x) + (vertexPositions[i].y - vertexPositions[i + 1].y) * (vertexPositions[i].y - vertexPositions[i + 1].y));

			for (size_t i = 0; i <= vertexPositions.size(); ++i)
			{
				float SideTexX;
				int idx = i < vertexPositions.size() ? i : 0;

				for (int j = 0; j < 2; ++j)
				{
					if (i != 0 && j == 0)
						CurrLength += std::sqrtf((vertexPositions[idx].x - vertexPositions[i - 1].x) * (vertexPositions[idx].x - vertexPositions[i - 1].x) + (vertexPositions[idx].y - vertexPositions[i - 1].y) * (vertexPositions[idx].y - vertexPositions[i - 1].y));

					if (j == 0)
						SideTexX = m_SideTextureLeft + (CurrLength / SideLength) * (m_SideTextureRight - m_SideTextureLeft);

					TexCoordBuffer.push_back(SideTexX);
					TexCoordBuffer.push_back(j == 0 ? m_SideTextureTop : m_SideTextureBottom);
				}
			}

			if (m_Offset == 0)
				m_Offset = TexCoordBuffer.size();

		}

	}

	glGenBuffers(1, (GLuint*)&m_TextureCoordBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, TexCoordBuffer.size() * sizeof(GLfloat), &TexCoordBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
