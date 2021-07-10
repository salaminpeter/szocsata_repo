#include "stdafx.h"
#include "UITileCounter.h"
#include "UIText.h"

#include <sstream>

CUITileCounter::CUITileCounter(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy) :
	CUIPanel(parent, L"ui_tile_counter", positionData, colorData, gridColorData, x, y, w, h, vx, vy, "tilecounter.bmp", 0.f, 0.f)
{
	AddText(L"", 0.f, 0.f, 50.f, 50.f, "font.bmp", L"ui_tile_counter_text");
}

void CUITileCounter::SetPositionAndSize(float x, float y, float w, float h)
{
	SetPosAndSize(x, y, w, h);
	CUIElement* Counter = GetChild(L"ui_tile_counter_text");
	Counter->SetPosAndSize(0, 0, 50, 50);
}

void CUITileCounter::SetCounter(unsigned count)
{
	std::wstringstream StrStream;

	StrStream << count;
	static_cast<CUIText*>(m_Children[0])->SetText(StrStream.str().c_str());  //TODO Legyen egy gettext/getbutton fuggveny!!!!!!!!!!!!
}

