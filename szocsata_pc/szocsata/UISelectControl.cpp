#include "stdafx.h"
#include "UISelectControl.h"
#include "UIButton.h"
#include "UIText.h"


CUISelectControl::CUISelectControl(CUIElement* parent, const wchar_t* id, const wchar_t* description, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, size_t idx) :
	CUIPanel(parent, id, positionData, colorData, gridcolorData, x, y, w, h, vx, vy, "", 0.f, 0.f),
	m_CurrSelection(idx)
{
	float TextSize = h * .6f;
	float DescTextSize = h * .45f;
	float IconSize = h * .75f;
	float DescTextWidth = CUIText::GetTextWidthInPixels(description, DescTextSize);
	glm::vec2 DescTextTopBottom = CUIText::GetTextTopBottom(description, DescTextSize);
	float DescTextYGap = DescTextSize / 5;

	SetPosAndSize(x, y, w, h + DescTextSize + DescTextYGap);

	CUIPanel* SelectPanel = new CUIPanel(this, L"ui_select_control_panel", positionData, colorData, gridcolorData, 0, (h - m_Height) / 2, w, h, vx, vy, "select_control_texture_generated", 0, 0);

	SelectPanel->AddText(description, -(w - DescTextWidth) / 2 + h / 3, h / 2 + DescTextSize / 2 - DescTextTopBottom.y + DescTextYGap, DescTextSize, "font.bmp", L"ui_select_control_desc_text");

	SelectPanel->AddText(L"", 0.f, 0.f, TextSize, "font.bmp", L"ui_select_control_text");
	static_cast<CUIText*>(GetChild(L"ui_select_control_text"))->Align(CUIText::Center);

	SelectPanel->AddButton(-(w - IconSize - IconSize / 3) / 2.f, 0.f, IconSize, IconSize, "left_arrow_icon.bmp", L"arrow_left");
	GetChild(L"arrow_left")->SetEvent(false, this, &CUISelectControl::ChangeEvent, -1);

	SelectPanel->AddButton((w - IconSize - IconSize / 3) / 2.f, 0, IconSize, IconSize, "right_arrow_icon.bmp", L"arrow_right");
	GetChild(L"arrow_right")->SetEvent(false, this, &CUISelectControl::ChangeEvent, 1);
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
