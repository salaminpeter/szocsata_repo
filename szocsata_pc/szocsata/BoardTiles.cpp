#include "stdafx.h"
#include "BoardTiles.h"
#include "Config.h"
#include "Model.h"
#include "RoundedBoxModelData.h"
#include "BoardTilesModelData.h"
#include "SquareModelData.h"


CBoardTiles::CBoardTiles(std::shared_ptr<CBoardTilesPositionData> positionData, std::shared_ptr<CBoardTilesTextureData> textureData, CModel* parent) :
	CModel(true, 3, std::static_pointer_cast<CModelPositionData>(positionData), std::static_pointer_cast<CModelColorData>(textureData), "gridtex.bmp", "per_pixel_light_textured")
{
	m_Parent = parent;

	float TileHeight;
	float BoardHeight;
	float BoardSize;
	int TileCount;
	float TileGap;

	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("board_size", BoardSize);

	ResetMatrix();
	Translate(glm::vec3(0.f, 0.f, BoardHeight / 2.f + TileHeight / 2.f));
	Scale(glm::vec3(1.f, 1.f, TileHeight));
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
