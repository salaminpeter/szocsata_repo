#pragma once

#include <memory>
#include <vector>
#include "glm\gtc\matrix_transform.hpp"

#include "Model.h"

class CRoundedBoxPositionData;
class CRenderer;
class CGameManager;

class CTileModel : public CModel
{
public:
	
	CTileModel(unsigned textureOffset, int bx, int by, std::shared_ptr<CRoundedBoxPositionData> positionData, std::shared_ptr<CModelColorData> colorData);

	unsigned TextureOffset() { return m_TextureOffset; }

private:

	int m_BoardX;
	int m_BoardY;

	unsigned m_TextureOffset;
};

class CBoardTiles
{
public:

	CBoardTiles(int tileCount, CRenderer* renderer, CGameManager* gameManager, CModel* parent);
	glm::vec2 GetTilePosition(int x, int y);
	void RenderTiles();

private:

	std::vector<CTileModel> m_BoardTiles;
	CRenderer* m_Renderer;
	CGameManager* m_GameManager;
};