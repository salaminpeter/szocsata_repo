#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "RoundedBoxModelData.h"
#include "Config.h"

#include <vector>
#include <numeric>

//TODO a RoundedSquare-t hasznalja ez is!
std::vector<glm::vec3> CRoundedBoxPositionData::m_TopVertices;

void CRoundedBoxPositionData::GenerateRoundedBoxVertices()
{
	if (m_TopVertices.size())
		return;

	float size = 1.f;

	m_TopVertices.reserve((m_SideLOD + 2) * 4);

	glm::vec3 ArchOrigo(-0.5f * size + m_SideRadius + m_EdgeRadius, -0.5f * size + m_SideRadius + m_EdgeRadius, 0.f);
	glm::vec3 ArchRadiusVec = glm::vec3(-m_SideRadius, 0.f, 0.f);

	float Angle = glm::radians(90.f / (m_SideLOD + 1));

	//top left corner
	for (int i = 0; i < m_SideLOD + 2; ++i)
	{
		glm::vec3 VertexPos = ArchRadiusVec + ArchOrigo;
		m_TopVertices.push_back(VertexPos);

		float NewX = ArchRadiusVec.x * std::cosf(Angle) - ArchRadiusVec.y * std::sinf(Angle);
		float NewY = ArchRadiusVec.x * std::sinf(Angle) + ArchRadiusVec.y * std::cosf(Angle);

		ArchRadiusVec.x = NewX;
		ArchRadiusVec.y = NewY;
	}
	
	//top right corner
	for (int i = m_SideLOD + 1; i >= 0; --i)
	{
		glm::vec3 VertexPos = m_TopVertices[i] * glm::vec3(-1.f, 1.f, 0.f);
		m_TopVertices.push_back(VertexPos);
	}
	
	//bottom right corner
	int From = m_TopVertices.size() - 1;
	for (int i = From; i >= From - m_SideLOD - 1; --i)
	{
		glm::vec3 VertexPos = m_TopVertices[i] * glm::vec3(1.f, -1.f, 0.f);
		m_TopVertices.push_back(VertexPos);
	}
	
	//bottom left corner
	From = m_TopVertices.size() - 1;
	for (int i = From; i >= From - m_SideLOD - 1; --i)
	{
		glm::vec3 VertexPos = m_TopVertices[i] * glm::vec3(-1.f, 1.f, 0.f);
		m_TopVertices.push_back(VertexPos);
	}
}

void CRoundedBoxPositionData::GeneratePositionBuffer()
{
	GenerateRoundedBoxVertices();

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
	glm::vec3 EdgeArchRadiusVec = glm::vec3(0.f, 0.f, m_EdgeRadius);
	float EdgeArchAngle = glm::radians(90.f / m_EdgeLod);

	for (size_t j = 0; j <= m_EdgeLod; ++j)
	{
		for (size_t i = 0; i < m_TopVertices.size(); ++i)
		{
			glm::vec2 OffsetVecDir = glm::normalize(glm::vec2(m_TopVertices[i].x, m_TopVertices[i].y)) * glm::length(glm::vec2(EdgeArchRadiusVec.x, EdgeArchRadiusVec.y));

			Vertices.push_back(m_TopVertices[i].x + OffsetVecDir.x);
			Vertices.push_back(m_TopVertices[i].y + OffsetVecDir.y);
			Vertices.push_back(0.5f - m_EdgeRadius  + EdgeArchRadiusVec.z);

			glm::vec3 Normal = glm::normalize(glm::vec3(OffsetVecDir.x, OffsetVecDir.y, EdgeArchRadiusVec.z));

			Vertices.push_back(Normal.x);
			Vertices.push_back(Normal.y);
			Vertices.push_back(Normal.z);
		}

		EdgeArchRadiusVec = glm::vec3(0.f, 0.f, m_EdgeRadius);
		glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), EdgeArchAngle * (j + 1), glm::vec3(0, 1, 0));
		EdgeArchRadiusVec = glm::vec3(glm::vec4(EdgeArchRadiusVec, 1.0f) * rot_mat);
	}

	/*
	//bottom vertex attributes
	int Offset = Vertices.size() - m_TopVertices.size() * 6;
	std::vector<float> BottomVertices;

	for (size_t i = Offset; i < Vertices.size(); i += 6)
	{
		BottomVertices.push_back(Vertices[i]);
		BottomVertices.push_back(Vertices[i + 1]);
		BottomVertices.push_back(-0.5f);

		BottomVertices.push_back(Vertices[i + 3]);
		BottomVertices.push_back(Vertices[i + 4]);
		BottomVertices.push_back(0.f);
	}

	Vertices.insert(Vertices.end(), std::make_move_iterator(BottomVertices.begin()), std::make_move_iterator(BottomVertices.end()));
	*/
	std::vector<unsigned int> Indices;

	//top indices
	for (size_t i = 1; i < m_TopVertices.size(); ++i)
	{
		Indices.push_back(i + 1);
		Indices.push_back(i);
		Indices.push_back(0);
	}

	
	Indices.push_back(1);
	Indices.push_back(m_TopVertices.size());
	Indices.push_back(0);
	
	//rounded edge indices
	for (size_t j = 0; j < m_EdgeLod; ++j)
	{
		int CurrRowOffset = j * m_TopVertices.size() + 1;
		int NextRowOffset = (j + 1) * m_TopVertices.size() + 1;

		for (size_t i = 0; i < m_TopVertices.size() - 1; ++i)
		{
			Indices.push_back(i + CurrRowOffset);
			Indices.push_back(i + CurrRowOffset + 1);
			Indices.push_back(i + NextRowOffset);

			Indices.push_back(i + NextRowOffset + 1);
			Indices.push_back(i + NextRowOffset);
			Indices.push_back(i + CurrRowOffset + 1);
		}

		Indices.push_back(CurrRowOffset);
		Indices.push_back(NextRowOffset + 1 + m_TopVertices.size() - 2);
		Indices.push_back(CurrRowOffset + 1 + m_TopVertices.size() - 2);

		Indices.push_back(CurrRowOffset);
		Indices.push_back(NextRowOffset);
		Indices.push_back(NextRowOffset + 1 + m_TopVertices.size() - 2);
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
	int SideLOD;
	int EdgeLOD;
	float SideRadius;
	float EdgeRadius;
	
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("letter_side_lod", SideLOD);
	CConfig::GetConfig("letter_side_radius", SideRadius);
	CConfig::GetConfig("letter_edge_lod", EdgeLOD);
	CConfig::GetConfig("letter_edge_radius", EdgeRadius);


	//TODO a tile texturanal az y - ok tukrozve vannak - javitani (meg kell nezni egy szabalyos texturaval hogy latszodjon a hiba)
	//TODO reserve texcoordbuffer
	std::vector<float> TexCoordBuffer;

	float TextWidth = (m_TopTextureRight - m_TopTextureLeft) / m_GridXSize;
	float TextHeight = (m_TopTextureTop - m_TopTextureBottom) / m_GridYSize;

	//calculate top texture distances
	glm::vec3 EdgeArchRadiusVec = glm::vec3(0.f, 0.f, EdgeRadius);
	float EdgeArchAngle = glm::radians(90.f / EdgeLOD);

	std::vector<std::vector<float>> DistList(vertexPositions.size());
	std::vector<float> DistFromOrigo;
	std::vector<glm::vec3> PrevVertexList;
	std::vector<glm::vec3> PrevVertexListTmp;


	for (size_t j = 0; j <= EdgeLOD; ++j)
	{
		PrevVertexListTmp = PrevVertexList;
		PrevVertexList.clear();

		for (size_t i = 0; i < vertexPositions.size(); ++i)
		{
			glm::vec3 PrevVertex;

			if (j == 0)
				PrevVertex = glm::vec3(0.f, 0.f, .5f);
			else
				PrevVertex = PrevVertexListTmp[i];

			glm::vec2 OffsetVecDir = glm::normalize(glm::vec2(vertexPositions[i].x, vertexPositions[i].y)) * glm::length(glm::vec2(EdgeArchRadiusVec.x, EdgeArchRadiusVec.y));
			glm::vec3 CurrVertex(vertexPositions[i].x + OffsetVecDir.x, vertexPositions[i].y + OffsetVecDir.y, 0.5f - EdgeRadius + EdgeArchRadiusVec.z);
			float Dist = glm::length(CurrVertex - PrevVertex);
			float PrevDist = DistList[i].size() ? DistList[i].back() : 0.f;
			DistList[i].push_back(PrevDist + Dist);
			PrevVertexList.push_back(CurrVertex);

			//create DistFromOrigo list
			if (j == EdgeLOD)
			{
				OffsetVecDir = glm::normalize(glm::vec2(vertexPositions[i].x, vertexPositions[i].y)) * glm::length(glm::vec2(EdgeArchRadiusVec.x, EdgeArchRadiusVec.y));
				DistFromOrigo.push_back(glm::length(glm::vec2(vertexPositions[i].x, vertexPositions[i].y) + OffsetVecDir));
			}
		}

		EdgeArchRadiusVec = glm::vec3(0.f, 0.f, EdgeRadius);
		glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), EdgeArchAngle * (j + 1), glm::vec3(0, 1, 0));
		EdgeArchRadiusVec = glm::vec3(glm::vec4(EdgeArchRadiusVec, 1.0f) * rot_mat);

	}


	for (unsigned y = 0; y < m_GridYSize; ++y) 
	{
		for (unsigned x = 0; x < m_GridXSize; ++x)
		{
			//tex coordinates for midle of top side
			TexCoordBuffer.push_back(m_TopTextureLeft + x * TextWidth + TextWidth / 2.f);
			TexCoordBuffer.push_back(m_TopTextureBottom + y * TextHeight + TextHeight / 2.f);

			//tex coordinates for top
			for (size_t j = 0; j <= EdgeLOD; ++j)
			{
				for (size_t i = 0; i < vertexPositions.size(); ++i)
				{
					float DistSum = DistList[i].back();
					float CurrDist = (DistList[i][j] / DistSum) * DistFromOrigo[i];
					glm::vec2 OffsetVecDir = glm::normalize(glm::vec2(vertexPositions[i].x, vertexPositions[i].y)) * CurrDist;

					TexCoordBuffer.push_back(m_TopTextureLeft + float(x) * TextWidth + (OffsetVecDir.x + .5f) * TextWidth);
					TexCoordBuffer.push_back(m_TopTextureBottom + float(y) * TextHeight +  (OffsetVecDir.y + .5f) * TextHeight);
				}
			}
			if (m_Offset == 0)
				m_Offset = TexCoordBuffer.size();

			/*
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
*/
		}

	}

	glGenBuffers(1, (GLuint*)&m_TextureCoordBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, TexCoordBuffer.size() * sizeof(GLfloat), &TexCoordBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
