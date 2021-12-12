#pragma once

#include "UIIconTextButton.h"

class CUIElement;

class CUITileCounter : public CUIIconTextButton
{
public:

	CUITileCounter(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy);
	void SetCounter(unsigned count);
	void SetCounter();
	void SetPositionAndSize(float x, float y, float w, float h);

	int GetCount() {return m_Count;}

private:

	int m_Count;
};
