#pragma once

#include "UIPanel.h"

class CGameManager;

class CUIRankingsPanel : public CUIPanel
{
public:

	CUIRankingsPanel(CUIElement* parent, CGameManager* gameManager, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty);

	void Init();
	void FinishGame();

private:

	CGameManager* m_GameManager;
};