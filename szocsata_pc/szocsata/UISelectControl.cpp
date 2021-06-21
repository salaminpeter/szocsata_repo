#include "stdafx.h"
#include "UISelectControl.h"
#include "UIButton.h"
#include "UIText.h"


CUISelectControl::CUISelectControl(CUIElement* parent, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, size_t idx) :
	CUIPanel(parent, id, positionData, colorData, gridcolorData, x, y, w, h, vx, vy, "selectcontrol.bmp", 0.f, 0.f),
	m_CurrSelection(idx)
{
	AddText(L"", -(w + h) / 2.f + h, 0.f, h - 20.f, h - 20.f, "font.bmp", L"ui_select_control_text");

	AddButton(-(w + h) / 2.f, 0.f, h, h, "leftarrow.bmp", L"arrow_left");
	m_Children.back()->SetEvent(this, &CUISelectControl::ChangeEvent, -1);

	AddButton((w + h) / 2.f, 0, h, h, "rightarrow.bmp", L"arrow_right");
	m_Children.back()->SetEvent(this, &CUISelectControl::ChangeEvent, 1);
}

void CUISelectControl::ChangeEvent(int dir)
{
	if (dir < 0 && m_CurrSelection == 0)
		m_CurrSelection = 0;
	else if (dir > 0 && m_CurrSelection >= m_Options.size() - 1)
		m_CurrSelection = m_Options.size() - 1;
	else
		m_CurrSelection += dir;

	static_cast<CUIText*>(GetChild(L"ui_select_control_text"))->SetText(m_Options[m_CurrSelection].c_str());
}

void CUISelectControl::SetIndex(size_t idx)
{
	m_CurrSelection = idx;
	static_cast<CUIText*>(GetChild(L"ui_select_control_text"))->SetText(m_Options[m_CurrSelection].c_str());
}


bool CUISelectControl::HandleEventAtPos(int x, int y) 
{
	for (size_t i = 1; i < 3; ++i)
	{
		if (m_Children[i]->PositionInElement(x, y))
		{
			m_Children[i]->HandleEvent();
			return true;
		}
	}

	return false;
}

void CUISelectControl::Render(CRenderer* renderer)
{
	CUIPanel::Render(renderer); //TODO a render virtualis fg kene hogy legyen!!!!
}

void CUISelectControl::AddOption(const wchar_t* text, bool setText) 
{ 
	m_Options.push_back(text); 

	if (setText)
		static_cast<CUIText*>(GetChild(L"ui_select_control_text"))->SetText(text);
}
