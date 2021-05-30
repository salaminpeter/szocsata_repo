#include "stdafx.h"
#include "BoardTilesModelData.h"
#include "SquareModelData.h"
#include "Config.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>


CBoardTilesPositionData::CBoardTilesPositionData(std::shared_ptr<CRoundedSquarePositionData> posData) :
	m_RoundedSquarePositionData(posData)
{
}

void CBoardTilesPositionData::GeneratePositionBuffer()
{
	float TileGap;
	float BoardHeight;
	float BoardSize;
	int TileCount;
	float TileHeight;

	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("board_height", BoardHeight);

	float GridScale = (2 * BoardSize - ((TileCount + 1) * TileGap)) / TileCount;

	BoardHeight /= 2.f;

	float SquareSize = (2 * BoardSize - ((TileCount + 1) * TileGap)) / TileCount;
	float XPos = -BoardSize;
	float YPos = -BoardSize;

	CConfig::AddConfig("tile_size", SquareSize);

	std::vector<glm::vec2>& SideNormals = m_RoundedSquarePositionData->GetNormals();
	std::vector<glm::vec3>& TopVertices = m_RoundedSquarePositionData->GetVertices();

	std::vector<unsigned int> Indices;
	std::vector<float> Vertices; //TODO reserve
	
	size_t VertexOffset = 0;

	for (int y = 0; y < TileCount; ++y)
	{
		YPos += TileGap;
		XPos = -BoardSize;

		for (int x = 0; x < TileCount; ++x)
		{
			XPos += TileGap;
			
			//origo
			Vertices.push_back(XPos + SquareSize / 2);
			Vertices.push_back(YPos + SquareSize / 2);
			Vertices.push_back(0.5f);

			Vertices.push_back(0.f);
			Vertices.push_back(0.f);
			Vertices.push_back(1.f);

			//top vertex attributes
			for (size_t i = 0; i < TopVertices.size(); ++i)
			{
				Vertices.push_back(TopVertices[i].x * GridScale + XPos + SquareSize / 2);
				Vertices.push_back(TopVertices[i].y * GridScale + YPos + SquareSize / 2);
				Vertices.push_back(0.5f);

				Vertices.push_back(0.f);
				Vertices.push_back(0.f);
				Vertices.push_back(1.f);
			}

			
			//side vertex attributes
			for (size_t i = 0; i <= TopVertices.size(); ++i)
			{
				float SideTexX;
				int idx = i < TopVertices.size() ? i : 0;

				for (int j = 0; j < 2; ++j)
				{
					Vertices.push_back(TopVertices[idx].x * GridScale + XPos + SquareSize / 2);
					Vertices.push_back(TopVertices[idx].y * GridScale + YPos + SquareSize / 2);
					Vertices.push_back(j == 0 ? 0.5f : -0.5f);

					Vertices.push_back(SideNormals[idx].x);
					Vertices.push_back(SideNormals[idx].y);
					Vertices.push_back(0.f);
				}
			}
			

			//top indices
			for (size_t i = 1; i < TopVertices.size(); ++i)
			{
				Indices.push_back(i + 1 + VertexOffset);
				Indices.push_back(i + VertexOffset);
				Indices.push_back(VertexOffset);
			}

			Indices.push_back(1 + VertexOffset);
			Indices.push_back(TopVertices.size() + VertexOffset);
			Indices.push_back(VertexOffset);

			
			//side indices
			int Offset = TopVertices.size() + 1;
			int fff = (TopVertices.size() - 1) * 2 + 2;

			for (size_t i = 0; i < (TopVertices.size() - 1) * 2 + 2; ++i)
			{
				Indices.push_back(VertexOffset + ((i % 2) ? Offset + i : Offset + i + 2));
				Indices.push_back(VertexOffset + Offset + i + 1);
				Indices.push_back(VertexOffset + ((i % 2) ? Offset + i + 2 : Offset + i));
			}
			
			VertexOffset = Vertices.size() / 6;
			XPos += SquareSize;
		}

		YPos += SquareSize;
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


void CBoardTilesTextureData::GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);


	//TODO a tile texturanal az y - ok tukrozve vannak - javitani (meg kell nezni egy szabalyos texturaval hogy latszodjon a hiba)
	//TODO reserve texcoordbuffer
	std::vector<float> TexCoordBuffer;

	float TextWidth = 1.f / TileCount;
	float TextHeight = 1.f / TileCount;

	for (unsigned y = 0; y < TileCount; ++y)
	{
		for (unsigned x = 0; x < TileCount; ++x)
		{
			//tex coordinates for midle of top side
			TexCoordBuffer.push_back(x * TextWidth + TextWidth / 2.f);
			TexCoordBuffer.push_back(y * TextHeight + TextHeight / 2.f);

			//tex coordinates for top
			for (size_t i = 0; i < vertexPositions.size(); ++i)
			{
				TexCoordBuffer.push_back(x * TextWidth + TextWidth * (vertexPositions[i].x + .5f));
				TexCoordBuffer.push_back(y * TextHeight + TextHeight * (vertexPositions[i].y + .5f));
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
						SideTexX = (CurrLength / SideLength) * 0.2f;

					TexCoordBuffer.push_back(SideTexX);
					TexCoordBuffer.push_back(j == 0 ? 0.1f : 0.f);
				}
			}
			
		}

	}

	glGenBuffers(1, (GLuint*)&m_TextureCoordBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBufferId);
	glBufferData(GL_ARRAY_BUFFER, TexCoordBuffer.size() * sizeof(GLfloat), &TexCoordBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

