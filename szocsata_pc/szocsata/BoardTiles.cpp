#include "stdafx.h"
#include "BoardTiles.h"
#include "Config.h"
#include "Model.h"
#include "Renderer.h"
#include "GameManager.h"
#include "RoundedBoxModelData.h"
#include "SquareModelData.h"


CTileModel::CTileModel(unsigned textureOffset, std::shared_ptr<CRoundedBoxPositionData> positionData, std::shared_ptr<CModelColorData> colorData) :
	CModel(true, 3, positionData, colorData, "gridtex.bmp", "per_pixel_light_textured", textureOffset)
{
}

CTileShadowModel::CTileShadowModel(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CModelColorData> colorData) :
	CModel(false, 3, positionData, colorData, "shadow.bmp", "textured")
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
	m_TileShadows.reserve(TileCount * TileCount);

	for (size_t y = 0; y < TileCount; ++y)
	{ 
		for (size_t x = 0; x < TileCount; ++x)
		{
			//add tile
			float Offset = renderer->GetTileColorData()->m_Offset;
			m_BoardTiles.emplace_back(Offset * y * TileCount + Offset * x, renderer->GetTilePositionData(), renderer->GetTileColorData());
			m_BoardTiles.back().SetParent(parent);
			m_BoardTiles.back().ResetMatrix();

			float Size = TileSize;
			float PosX = -BoardSize + TileGap + TileGap * x + TileSize * x + Size / 2;
			float PosY = -BoardSize + TileGap + TileGap * y + TileSize * y + Size / 2;
			float PosZ = BoardHeight - 0.1; //TODO mi a lofasz ez a -0.1????

			m_BoardTiles.back().SetPosition(glm::vec3(PosX, PosY, PosZ));
			
			//add shadow
			m_TileShadows.emplace_back(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData());
			m_TileShadows.back().SetParent(parent);
			m_TileShadows.back().ResetMatrix();

			PosX = -BoardSize + TileGap + TileGap * x + TileSize * x + Size / 2;
			PosY = -BoardSize + TileGap + TileGap * y + TileSize * y + Size / 2;
			PosZ = BoardHeight / 2;

			float Scale = TileSize + TileGap;
			m_TileShadows.back().Scale(glm::vec3(Scale, Scale, 1.f));
			m_TileShadows.back().SetPosition(glm::vec3(PosX, PosY, PosZ));
		}
	}
}

bool CBoardTiles::IsTileVisible(int x, int y)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	return m_BoardTiles[y * TileCount + x].IsVisible();
}

void CBoardTiles::SetTileVisible(int x, int y, bool visible)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	m_BoardTiles[y * TileCount + x].SetVisible(visible);
}


void CBoardTiles::RenderTiles()
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	glDisable(GL_DEPTH);
	glEnable(GL_BLEND);

	m_Renderer->SetTexturePos(glm::vec2(0.f, 0.f));
	m_Renderer->SetModifyColor(1, 1, 1, 1);

	for (size_t i = 0; i < m_TileShadows.size(); ++i)
		m_Renderer->DrawModel(&m_TileShadows[i], "board_perspecive", "textured", false);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH);

	int LastVisibleTileIdx = 0;

	for (size_t i = 0; i < m_BoardTiles.size(); ++i)
	{
		int x = i % TileCount;
		int y = i / TileCount;

		if (m_GameManager->GetRenderer()->IsTileVisible(x, y))
			LastVisibleTileIdx = i;
	}

	bool BufferBound = false;
	bool TextureBound = false;

	for (size_t i = 0; i < m_BoardTiles.size(); ++i)
	{
		int x = i % TileCount;
		int y = i / TileCount;

		if (m_BoardTiles[i].IsVisible())
		{
			m_Renderer->DrawModel(&m_BoardTiles[i], "board_perspecive", "per_pixel_light_textured", true, !BufferBound, !TextureBound, i == LastVisibleTileIdx, true);

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
