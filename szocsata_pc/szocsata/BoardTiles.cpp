#include "stdafx.h"
#include "BoardTiles.h"
#include "Config.h"
#include "Model.h"
#include "Renderer.h"
#include "GameManager.h"
#include "RoundedBoxModelData.h"
#include "SquareModelData.h"


CTileModel::CTileModel(unsigned textureOffset, int bx, int by, std::shared_ptr<CRoundedBoxPositionData> positionData, std::shared_ptr<CModelColorData> colorData) :
	CModel(true, 3, positionData, colorData, "gridtex.bmp", "per_pixel_light_textured"),
	m_BoardX(bx),
	m_BoardY(by),
	m_TextureOffset(textureOffset)
{
}

CBoardTiles::CBoardTiles(int tileCount, CRenderer* renderer, CGameManager* gameManager, CModel* parent) : 
	m_Renderer(renderer),
	m_GameManager(gameManager)
{
	int TileCount;
	float TileGap;
	float TileSize;
	float BoardSize;
	float BoardHeight;

	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("tile_size", TileSize);

	m_BoardTiles.reserve(TileCount * TileCount);

	for (size_t y = 0; y < TileCount; ++y)
	{ 
		for (size_t x = 0; x < TileCount; ++x)
		{
			float Offset = renderer->GetTileColorData()->m_Offset;
			m_BoardTiles.emplace_back(Offset * y * TileCount + Offset * x, 0, 0, renderer->GetTilePositionData(), renderer->GetTileColorData());
			m_BoardTiles.back().SetParent(parent);
			m_BoardTiles.back().ResetMatrix();

			float Size = TileSize;
			float SelectionX = -BoardSize + TileGap + TileGap * x + TileSize * x + Size / 2 - 0.02;
			float SelectionY = -BoardSize + TileGap + TileGap * y + TileSize * y + Size / 2 - 0.02;
			float SelectionZ = BoardHeight;

			m_BoardTiles.back().Translate(glm::vec3(SelectionX, SelectionY, SelectionZ));
		}
	}
}

void CBoardTiles::RenderTiles()
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	int LastVisibleTileIdx = 0;

	for (size_t i = 0; i < m_BoardTiles.size(); ++i)
	{
		int x = i % TileCount;
		int y = i / TileCount;

		if (m_GameManager->Board(x, TileCount - y - 1).m_Height == 0)
			LastVisibleTileIdx = i;
	}

	bool BufferBound = false;
	bool TextureBound = false;

	for (size_t i = 0; i < m_BoardTiles.size(); ++i)
	{
		int x = i % TileCount;
		int y = i / TileCount;

		if (m_GameManager->Board(x, TileCount - y - 1).m_Height == 0)
		{
			m_Renderer->DrawModel(&m_BoardTiles[i], "board_perspecive", "per_pixel_light_textured", true, !BufferBound, !TextureBound, i == LastVisibleTileIdx, true, m_BoardTiles[i].TextureOffset());

			if (i == LastVisibleTileIdx)
				return;

			BufferBound = true;
			TextureBound = true;
		}
	}
}

glm::vec2 CBoardTiles::GetTilePosition(int x, int y)
{
	float TileGap;
	float BoardSize;
	int TileCount;

	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("board_size", BoardSize);

	float TileSize = (2 * BoardSize - ((TileCount + 1) * TileGap)) / TileCount;
	float XPos = -BoardSize + TileSize / 2 + (x * (TileGap + TileSize));
	float YPos = -BoardSize + TileSize / 2 + (y * (TileGap + TileSize));

	return glm::vec2(XPos, YPos);
}
