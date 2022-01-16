#pragma once

#include "UIPanel.h"

class CGameManager;

class CUIScorePanel : public CUIPanel
{
public:

	CUIScorePanel(CUIElement* parent, CGameManager* gameManager, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty);

	void Update();
	void Init();

protected:

	virtual void ResizeElement(float widthPercent, float heightPercent) override;

private:

	CGameManager* m_GameManager;
	float m_ScoreXPosition;
};