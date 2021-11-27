#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "RoundedBoxModelData.h"
#include "Config.h"

#include <vector>
#include <numeric>

void CRoundedBoxPositionData::GenerateRoundedBoxVertices()
{
	m_TopVerticesInner.reserve((m_SideLOD + 2) * 4);
	m_TopVerticesOuter.reserve((m_SideLOD + 2) * 4);

	glm::vec2 ArchOrigo(-0.5f * m_Size	+ m_SideRadius + m_EdgeRadius, -0.5f * m_Size + m_SideRadius + m_EdgeRadius);
	glm::vec2 ArchRadiusVec = glm::vec2(-m_SideRadius, 0.f);

	float Angle = glm::radians(90.f / (m_SideLOD + 1));

	//top left corner
	for (int i = 0; i < m_SideLOD + 2; ++i)
	{
		glm::vec2 VertexPos = ArchRadiusVec + ArchOrigo;
		m_TopVerticesInner.push_back(VertexPos);

		float NewX = ArchRadiusVec.x * std::cosf(Angle) - ArchRadiusVec.y * std::sinf(Angle);
		float NewY = ArchRadiusVec.x * std::sinf(Angle) + ArchRadiusVec.y * std::cosf(Angle);

		ArchRadiusVec.x = NewX;
		ArchRadiusVec.y = NewY;
	}
	
	//top right corner
	for (int i = m_SideLOD + 1; i >= 0; --i)
	{
		glm::vec2 VertexPos = m_TopVerticesInner[i] * glm::vec2(-1.f, 1.f);
		m_TopVerticesInner.push_back(VertexPos);
	}
	
	//bottom right corner
	int From = m_TopVerticesInner.size() - 1;
	for (int i = From; i >= From - m_SideLOD - 1; --i)
	{
		glm::vec2 VertexPos = m_TopVerticesInner[i] * glm::vec2(1.f, -1.f);
		m_TopVerticesInner.push_back(VertexPos);
	}
	
	//bottom left corner
	From = m_TopVerticesInner.size() - 1;
	for (int i = From; i >= From - m_SideLOD - 1; --i)
	{
		glm::vec2 VertexPos = m_TopVerticesInner[i] * glm::vec2(-1.f, 1.f);
		m_TopVerticesInner.push_back(VertexPos);
	}

	//set outer topvertices
	for (size_t i = 0; i < m_TopVerticesInner.size(); ++i)
	{
		float InnerVertexDist = glm::length(m_TopVerticesInner[i]);
		glm::vec2 OrigoToVertexVec(glm::normalize(m_TopVerticesInner[i]) * (InnerVertexDist + m_EdgeRadius));
		m_TopVerticesOuter.push_back(OrigoToVertexVec);
	}
}

void CRoundedBoxPositionData::GeneratePositionBuffer()
{
	GenerateRoundedBoxVertices();

	std::vector<float> Vertices;
	//TODO reserve

	float Height = m_Height / 2;
	
	//origo
	Vertices.push_back(0.f);
	Vertices.push_back(0.f);
	Vertices.push_back(Height);

	Vertices.push_back(0.f);
	Vertices.push_back(0.f);
	Vertices.push_back(1.f);


	//top vertex attributes
	glm::vec3 EdgeArchRadiusVec = glm::vec3(0.f, 0.f, m_EdgeRadius);
	float EdgeArchAngle = glm::radians(90.f / m_EdgeLOD);

	for (size_t j = 0; j <= m_EdgeLOD; ++j)
	{
		for (size_t i = 0; i < m_TopVerticesInner.size(); ++i)
		{
			glm::vec2 OffsetVecDir = glm::normalize(glm::vec2(m_TopVerticesInner[i].x, m_TopVerticesInner[i].y)) * glm::length(glm::vec2(EdgeArchRadiusVec.x, EdgeArchRadiusVec.y));

			Vertices.push_back(m_TopVerticesInner[i].x + OffsetVecDir.x);
			Vertices.push_back(m_TopVerticesInner[i].y + OffsetVecDir.y);
			Vertices.push_back(Height - m_EdgeRadius + EdgeArchRadiusVec.z);

			glm::vec3 Normal = glm::normalize(glm::vec3(OffsetVecDir.x, OffsetVecDir.y, EdgeArchRadiusVec.z));

			Vertices.push_back(Normal.x);
			Vertices.push_back(Normal.y);
			Vertices.push_back(Normal.z);
		}

		EdgeArchRadiusVec = glm::vec3(0.f, 0.f, m_EdgeRadius);
		glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), EdgeArchAngle * (j + 1), glm::vec3(0, 1, 0));
		EdgeArchRadiusVec = glm::vec3(glm::vec4(EdgeArchRadiusVec, 1.0f) * rot_mat);
	}

	
	//side vertex attributes
	int Offset = Vertices.size() - m_TopVerticesInner.size() * 6;
	std::vector<float> TopVertices;
	std::vector<float> BottomVertices;

	for (size_t i = Offset; i < Vertices.size(); i += 6)
	{
		BottomVertices.push_back(Vertices[i]);
		BottomVertices.push_back(Vertices[i + 1]);
		BottomVertices.push_back(-Height);
		BottomVertices.push_back(Vertices[i + 3]);
		BottomVertices.push_back(Vertices[i + 4]);
		BottomVertices.push_back(0.f);

		TopVertices.push_back(Vertices[i]);
		TopVertices.push_back(Vertices[i + 1]);
		TopVertices.push_back(Height - m_EdgeRadius);
		TopVertices.push_back(Vertices[i + 3]);
		TopVertices.push_back(Vertices[i + 4]);
		TopVertices.push_back(0.f);
	}

	BottomVertices.push_back(Vertices[Offset]);
	BottomVertices.push_back(Vertices[Offset + 1]);
	BottomVertices.push_back(-Height);
	BottomVertices.push_back(Vertices[Offset + 3]);
	BottomVertices.push_back(Vertices[Offset + 4]);
	BottomVertices.push_back(0.f);

	TopVertices.push_back(Vertices[Offset]);
	TopVertices.push_back(Vertices[Offset + 1]);
	TopVertices.push_back(Height - m_EdgeRadius);
	TopVertices.push_back(Vertices[Offset + 3]);
	TopVertices.push_back(Vertices[Offset + 4]);
	TopVertices.push_back(0.f);


	Vertices.insert(Vertices.end(), std::make_move_iterator(TopVertices.begin()), std::make_move_iterator(TopVertices.end()));
	Vertices.insert(Vertices.end(), std::make_move_iterator(BottomVertices.begin()), std::make_move_iterator(BottomVertices.end()));
	
	std::vector<unsigned int> Indices;

	//top indices
	for (size_t i = 1; i < m_TopVerticesInner.size(); ++i)
	{
		Indices.push_back(i + 1);
		Indices.push_back(i);
		Indices.push_back(0);
	}
	
	Indices.push_back(1);
	Indices.push_back(m_TopVerticesInner.size());
	Indices.push_back(0);

	int CurrRowOffset;
	int NextRowOffset;
	
	//rounded edge indices
	for (size_t j = 0; j < m_EdgeLOD; ++j)
	{
		CurrRowOffset = j * m_TopVerticesInner.size() + 1;
		NextRowOffset = (j + 1) * m_TopVerticesInner.size() + 1;

		for (size_t i = 0; i < m_TopVerticesInner.size() - 1; ++i)
		{
			Indices.push_back(i + CurrRowOffset);
			Indices.push_back(i + CurrRowOffset + 1);
			Indices.push_back(i + NextRowOffset);

			Indices.push_back(i + NextRowOffset + 1);
			Indices.push_back(i + NextRowOffset);
			Indices.push_back(i + CurrRowOffset + 1);
		}

		Indices.push_back(CurrRowOffset);
		Indices.push_back(NextRowOffset + 1 + m_TopVerticesInner.size() - 2);
		Indices.push_back(CurrRowOffset + 1 + m_TopVerticesInner.size() - 2);

		Indices.push_back(CurrRowOffset);
		Indices.push_back(NextRowOffset);
		Indices.push_back(NextRowOffset + 1 + m_TopVerticesInner.size() - 2);
	}
	
	//side indices
	CurrRowOffset = (m_EdgeLOD + 1) * m_TopVerticesInner.size() + 1;
	NextRowOffset = (m_EdgeLOD + 2) * m_TopVerticesInner.size() + 2;

	for (size_t i = 0; i < m_TopVerticesInner.size(); ++i)
	{
		Indices.push_back(i + CurrRowOffset);
		Indices.push_back(i + CurrRowOffset + 1);
		Indices.push_back(i + NextRowOffset + 1);
		
		Indices.push_back(i + NextRowOffset + 1);
		Indices.push_back(i + NextRowOffset);
		Indices.push_back(i + CurrRowOffset);
		
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

std::vector<glm::vec3> CRoundedBoxPositionData::GetTopVertices(bool inner) 
{ 
	std::vector<glm::vec3> Ret;
	std::vector<glm::vec2>& TopVertices = inner ? m_TopVerticesInner : m_TopVerticesOuter;

	for (size_t i = 0; i < TopVertices.size(); ++i)
		Ret.push_back(glm::vec3(TopVertices[i], 0.f));

	return Ret; 
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
	//TODO reserve texcoordbuffer
	std::vector<float> TexCoordBuffer;

	float TextWidth = (m_TopTextureRight - m_TopTextureLeft) / m_GridXSize;
	float TextHeight = (m_TopTextureTop - m_TopTextureBottom) / m_GridYSize;

	//calculate top texture distances
	glm::vec3 EdgeArchRadiusVec = glm::vec3(0.f, 0.f, m_EdgeRadius);
	float EdgeArchAngle = glm::radians(90.f / m_EdgeLOD);

	std::vector<std::vector<float>> DistList(vertexPositions.size());
	std::vector<float> DistFromOrigo;
	std::vector<glm::vec3> PrevVertexList;
	std::vector<glm::vec3> PrevVertexListTmp;

	for (size_t j = 0; j <= m_EdgeLOD; ++j)
	{
		PrevVertexListTmp = PrevVertexList;
		PrevVertexList.clear();

		for (size_t i = 0; i < vertexPositions.size() / 2; ++i)
		{
			glm::vec3 PrevVertex;

			if (j == 0)
				PrevVertex = glm::vec3(0.f, 0.f, .5f);
			else
				PrevVertex = PrevVertexListTmp[i];

			glm::vec2 OffsetVecDir = glm::normalize(glm::vec2(vertexPositions[i].x, vertexPositions[i].y)) * glm::length(glm::vec2(EdgeArchRadiusVec.x, EdgeArchRadiusVec.y));
			glm::vec3 CurrVertex(vertexPositions[i].x + OffsetVecDir.x, vertexPositions[i].y + OffsetVecDir.y, 0.5f - m_EdgeRadius + EdgeArchRadiusVec.z);
			float Dist = glm::length(CurrVertex - PrevVertex);
			float PrevDist = DistList[i].size() ? DistList[i].back() : 0.f;
			DistList[i].push_back(PrevDist + Dist);
			PrevVertexList.push_back(CurrVertex);

			//create DistFromOrigo list
			if (j == m_EdgeLOD)
				DistFromOrigo.push_back(glm::length(vertexPositions[vertexPositions.size() / 2 + i]));
		}

		EdgeArchRadiusVec = glm::vec3(0.f, 0.f, m_EdgeRadius);
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
			for (size_t j = 0; j <= m_EdgeLOD; ++j)
			{
				for (size_t i = 0; i < vertexPositions.size() / 2; ++i)
				{
					float DistSum = DistList[i].back();
					float CurrDist = (DistList[i][j] / DistSum) * DistFromOrigo[i];
					glm::vec2 OffsetVecDir = glm::normalize(glm::vec2(vertexPositions[i].x, vertexPositions[i].y)) * CurrDist;

					TexCoordBuffer.push_back(m_TopTextureLeft + float(x) * TextWidth + (OffsetVecDir.x + .5f) * TextWidth);
					TexCoordBuffer.push_back(m_TopTextureBottom + float(y) * TextHeight +  (OffsetVecDir.y + .5f) * TextHeight);
				}
			}
			
			//tex coordinates for side
			std::vector<float> SideDistList;
			size_t VertexCount = vertexPositions.size() / 2;

			for (size_t i = 0; i <= VertexCount; ++i) {
				size_t VertexIdx0 = i == 0 ? 0 : i - 1;
				size_t VertexIdx1 = i == VertexCount ? 0 : i;

				glm::vec2 CurrVertex = vertexPositions[VertexIdx0 + VertexCount];
				glm::vec2 NextVertex = vertexPositions[VertexIdx1 + VertexCount];
				float PrevDist = SideDistList.size() ? SideDistList.back() : 0.f;
				SideDistList.push_back(PrevDist + glm::length(CurrVertex - NextVertex));
			}

			for (size_t i = 0; i <= VertexCount; ++i)
			{
				float SideTexX;
				
				SideTexX = m_SideTextureLeft + (SideDistList[i] / SideDistList.back()) * (m_SideTextureRight - m_SideTextureLeft);
				TexCoordBuffer.push_back(SideTexX);
				TexCoordBuffer.push_back(m_SideTextureTop);
			}

			for (size_t i = 0; i <= VertexCount; ++i)
			{
				float SideTexX;

				SideTexX = m_SideTextureLeft + (SideDistList[i] / SideDistList.back()) * (m_SideTextureRight - m_SideTextureLeft);
				TexCoordBuffer.push_back(SideTexX);
				TexCoordBuffer.push_back(m_SideTextureBottom);
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
