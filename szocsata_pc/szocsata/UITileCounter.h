#pragma once

#include "UIPanel.h"

class CUITileCounter : public CUIPanel
{
public:

	CUITileCounter(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy);
	void SetCounter(unsigned count);
};
