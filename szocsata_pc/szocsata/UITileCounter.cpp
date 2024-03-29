#include "stdafx.h"
#include "UITileCounter.h"
#include "UIPanel.h"
#include "UIText.h"

#include <sstream>

CUITileCounter::CUITileCounter(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy) :
	CUIIconTextButton(parent, L"", positionData, colorData, gridColorData, x, y, w, h, vx, vy, "tile_counter_texture_generated", "tile_counter_icon.bmp", L"ui_tile_counter", 1.f, "textured", .8f, .2f, 20.f)
{
	static_cast<CUIPanel*>(GetChild(L"ui_button_icon"))->AddText(L"0", 0, 0, w / 2, "font.bmp", L"ui_tile_counter_text", 0, 0, 0);
}

void CUITileCounter::ResizeElement(float widthPercent, float heightPercent)
{
	CUIText* Counter = static_cast<CUIText*>(GetChild(L"ui_tile_counter_text"));
	Counter->Align(CUIText::Center);
	CenterIcon();
}

void CUITileCounter::SetPositionAndSize(float x, float y, float w, float h)
{
	SetPosAndSize(x, y, w, h, false);
	CUIText* Counter = static_cast<CUIText*>(GetChild(L"ui_tile_counter_text"));
	Counter->Align(CUIText::Center);
	CenterIcon();
}

void CUITileCounter::SetCounter(unsigned count)
{
	m_Count = count;
	std::wstringstream StrStream;

	StrStream << count;

	CUIText* CounterText = static_cast<CUIText*>(GetChild(L"ui_tile_counter_text"));
	CounterText->SetText(StrStream.str().c_str());  //TODO Legyen egy gettext/getbutton fuggveny!!!!!!!!!!!!
}

void CUITileCounter::SetCounter()
{
	SetCounter(m_Count);
}
