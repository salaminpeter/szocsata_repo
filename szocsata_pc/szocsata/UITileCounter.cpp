#include "stdafx.h"
#include "UITileCounter.h"
#include "UIText.h"

#include <sstream>

CUITileCounter::CUITileCounter(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy) :
	CUIPanel(nullptr, L"ui_tile_counter", positionData, colorData, x, y, w, h, vx, vy, "tilecounter.bmp", 0.f, 0.f)
{
	AddText(L"", positionData, gridColorData, 0.f, 0.f, 50.f, 50.f, "font.bmp", L"ui_tile_counter_text");
}

void CUITileCounter::SetCounter(unsigned count)
{
	std::wstringstream StrStream;

	StrStream << count;
	static_cast<CUIText*>(m_Children[0])->SetText(StrStream.str().c_str());
}

