#include "stdafx.h"
#include "UISelectControl.h"
#include "UIButton.h"
#include "UIText.h"


CUISelectControl::CUISelectControl(CUIElement* parent, const wchar_t* id, const wchar_t* description, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, size_t idx) :
	CUIPanel(parent, id, positionData, colorData, gridcolorData, x, y, w, h, vx, vy, "select_control_texture_generated", 0.f, 0.f),
	m_CurrSelection(idx)
{
	float TextSize = h * .6f;
	float DescTextSize = h * .45f;
	float IconSize = h * .75f;

	glm::vec2 DescTextTopBottom = CUIText::GetTextTopBottom(description, DescTextSize);
	float DescTextWidth = CUIText::GetTextWidthInPixels(description, DescTextSize);

	AddText(description, -(w - DescTextWidth) / 2 + h / 3, h / 2 + DescTextSize / 2 - DescTextTopBottom.y + DescTextSize / 5, DescTextSize, "font.bmp", L"ui_select_control_desc_text");

	AddText(L"", 0.f, 0.f, TextSize, "font.bmp", L"ui_select_control_text");
	static_cast<CUIText*>(m_Children.back())->Align(CUIText::Center);

	AddButton(-(w - IconSize - IconSize / 3) / 2.f, 0.f, IconSize, IconSize, "left_arrow_icon.bmp", L"arrow_left");
	m_Children.back()->SetEvent(false, this, &CUISelectControl::ChangeEvent, -1);

	AddButton((w - IconSize - IconSize / 3) / 2.f, 0, IconSize, IconSize, "right_arrow_icon.bmp", L"arrow_right");
	m_Children.back()->SetEvent(false, this, &CUISelectControl::ChangeEvent, 1);
}

void CUISelectControl::SetTextAndPos(const wchar_t* text)
{
	CUIText* Text = static_cast<CUIText*>(GetChild(L"ui_select_control_text"));
	Text->SetText(text);
	Text->Align(CUIText::Center);
}


void CUISelectControl::ChangeEvent(int dir)
{
	if (dir < 0 && m_CurrSelection == 0)
		m_CurrSelection = 0;
	else if (dir > 0 && m_CurrSelection >= m_Options.size() - 1)
		m_CurrSelection = m_Options.size() - 1;
	else
		m_CurrSelection += dir;

	SetTextAndPos(m_Options[m_CurrSelection].c_str());
}

void CUISelectControl::SetIndex(size_t idx)
{
	m_CurrSelection = idx;
	SetTextAndPos(m_Options[m_CurrSelection].c_str());
}

void CUISelectControl::AddOption(const wchar_t* text, bool setText) 
{ 
	m_Options.push_back(text); 

	if (setText)
		SetTextAndPos(text);
}
