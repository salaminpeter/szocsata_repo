#include "stdafx.h"
#include "UISelectControl.h"
#include "UIButton.h"
#include "UIText.h"


CUISelectControl::CUISelectControl(CUIElement* parent, const wchar_t* id, const wchar_t* description, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, size_t idx) :
	CUIPanel(parent, id, positionData, colorData, gridcolorData, x, y, w, h, vx, vy, "", 0.f, 0.f),
	m_CurrSelection(idx)
{
	float DescTextSize = h * .3f;
	float ControlSize = h * .68f;
	float TextSize = ControlSize * .55f;
	float IconSize = ControlSize * .8f;

	float DescTextWidth = CUIText::GetTextWidthInPixels(description, DescTextSize);
	glm::vec2 DescTextTopBottom = CUIText::GetTextTopBottom(description, DescTextSize);
	float DescTextYGap = h * .02f;
	float ControlY = -(h - ControlSize) / 2;

	CUIPanel* SelectPanel = new CUIPanel(this, L"ui_select_control_panel", positionData, colorData, gridcolorData, 0, ControlY, w, ControlSize, vx, vy, "select_control_texture_generated", 0, 0);

	CUIText* DescText = SelectPanel->AddText(description, 0, 0, DescTextSize, "font.bmp", L"ui_select_control_desc_text");
	DescText->SetPosition(0, ControlSize + DescTextYGap - DescTextTopBottom.y, false);

	SelectPanel->AddText(L"", 0.f, 0.f, TextSize, "font.bmp", L"ui_select_control_text");
	static_cast<CUIText*>(GetChild(L"ui_select_control_text"))->Align(CUIText::Center);

	float ArrowHeight = IconSize;
	float ArrowWidth = ArrowHeight * .59f;

	CUIButton* Button = SelectPanel->AddButton(-(w - ArrowWidth - ArrowWidth / 2) / 2.f, 0.f, ArrowWidth, ArrowHeight, "left_arrow_icon.bmp", L"arrow_left");
	Button->SetEvent(CUIElement::ReleaseEvent, this, &CUISelectControl::ChangeEvent, -1);
	Button->SetTouchOffset(Button->GetWidth(), Button->GetHeight() / 2.f);

	Button = SelectPanel->AddButton((w - ArrowWidth - ArrowWidth / 2) / 2.f, 0, ArrowWidth, ArrowHeight, "right_arrow_icon.bmp", L"arrow_right");
	Button->SetEvent(CUIElement::ReleaseEvent, this, &CUISelectControl::ChangeEvent, 1);
	Button->SetTouchOffset(Button->GetWidth(), Button->GetHeight() / 2.f);
}

CUIElement* CUISelectControl::GetLeftButton()
{
	return GetChild(L"arrow_left");
}

CUIElement* CUISelectControl::GetRightButton()
{
	return GetChild(L"arrow_right");
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
