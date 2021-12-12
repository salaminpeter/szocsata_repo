#include "stdafx.h"
#include "UITileCounter.h"
#include "UIPanel.h"
#include "UIText.h"

#include <sstream>

CUITileCounter::CUITileCounter(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy) :
	CUIIconTextButton(parent, L"", positionData, colorData, gridColorData, x, y, w, h, vx, vy, "score_panel_texture_generated", "tile_counter_icon.bmp", L"ui_tile_counter", "textured", .8f, .2f, 20.f)
{
	static_cast<CUIPanel*>(GetChild(L"ui_button_icon"))->AddText(L"0", 0, 0, 70, "font.bmp", L"ui_tile_counter_text", 1, 1, 1);
}

void CUITileCounter::SetPositionAndSize(float x, float y, float w, float h)
{
	SetPosAndSize(x, y, w, h);
	CUIText* Counter = static_cast<CUIText*>(GetChild(L"ui_tile_counter_text"));
	Counter->Align(CUIText::Center);
}

void CUITileCounter::SetCounter(unsigned count)
{
	m_Count = count;
	std::wstringstream StrStream;

	StrStream << count;

	CUIText* CounterText = static_cast<CUIText*>(GetChild(L"ui_tile_counter_text"));
	CounterText->SetText(StrStream.str().c_str());  //TODO Legyen egy gettext/getbutton fuggveny!!!!!!!!!!!!
	CounterText->Align(CUIText::Center);
}

void CUITileCounter::SetCounter()
{
	SetCounter(m_Count);
}
