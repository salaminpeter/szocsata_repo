#pragma once

#include <memory>
#include <vector>
#include "glm\gtc\matrix_transform.hpp"

#include "Model.h"

class CRoundedBoxPositionData;
class CSquarePositionData;
class CRenderer;
class CGameManager;

class CTileModel : public CModel
{
public:
	
	CTileModel(unsigned textureOffset, std::shared_ptr<CRoundedBoxPositionData> positionData, std::shared_ptr<CModelColorData> colorData);

	void SetVisible(bool visible) {m_Visible = visible;	}
	bool IsVisible() {return m_Visible;}

private:

	bool m_Visible = true;
};


class CTileShadowModel : public CModel
{
public:

	CTileShadowModel(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CModelColorData> colorData);
};

class CBoardTiles
{
public:

	CBoardTiles(int tileCount, CRenderer* renderer, CGameManager* gameManager, CModel* parent);
	glm::vec2 GetTilePosition(int x, int y);
	void RenderTiles();
	void SetTileVisible(int x, int y, bool visible);

private:

	std::vector<CTileModel> m_BoardTiles;
	std::vector<CTileShadowModel> m_TileShadows;
	CRenderer* m_Renderer;
	CGameManager* m_GameManager;
};