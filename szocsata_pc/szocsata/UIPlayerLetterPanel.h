#pragma once

#include "UIPanel.h"

class CUIPlayerLetterPanel : public CUIPanel
{
public:

	CUIPlayerLetterPanel(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float viewX, float viewY) :
		CUIPanel(parent, L"ui_player_letter_panel", positionData, colorData, nullptr, x, y, w, h, viewX, viewY, "player_letter_panel_texture_generated", 0, 0)
	{}

	bool HandleEventAtPos(int x, int y, EEventType event, CUIElement* root = nullptr, bool checkChildren = true, bool selfCheck = true) override;
};