#pragma once

#include "UIPanel.h"

class CUIPlayerLetterPanel : public CUIPanel
{
public:

	CUIPlayerLetterPanel(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float viewX, float viewY) :
		CUIPanel(parent, L"ui_player_letter_panel", positionData, colorData, nullptr, 0, 0, 0, 0, viewX, viewY, "player_letter_panel_texture_generated", 0, 0)
	{}

	bool HandleEventAtPos(int x, int y, bool touchEvent, CUIElement* root, bool checkChildren) override;
};