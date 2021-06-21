#include "stdafx.h"
#include "UIMessageBox.h"
#include "UIText.h"

CUIMessageBox* CUIMessageBox::m_ActiveMessageBox = nullptr;
int CUIMessageBox::m_RetValue = false;


CUIMessageBox::CUIMessageBox(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type) :
	CUIPanel(nullptr, L"ui_message_box", positionData, colorData, gridColorData, x, y, w, h, vx, vy, "panel.bmp", 0.f, 0.f),
	m_Type(type)
{
	AddText(L"",-120.f, 0.f, 40.f, 40.f, "font.bmp", L"ui_message_box_text");

	if (m_Type == Ok)
	{ 
		int BtnX = m_Width / 2;
		int BtnY = m_Height ;
		AddButton(0, -100, 120, 120, "okbutton.bmp", L"ui_message_box_ok_btn");
		m_Children.back()->SetEvent(this, &CUIMessageBox::ButtonPressed, 1);
	}

	if (m_Type == OkCancel)
	{
		AddButton(80, -80, 50, 50, "cancelbutton.bmp", L"ui_message_box_cancel_btn");
		m_Children.back()->SetEvent(this, &CUIMessageBox::ButtonPressed, 0);
	}
}

void CUIMessageBox::ButtonPressed(int ret)
{
	m_RetValue = ret;
	m_ActiveMessageBox = nullptr;
}

void CUIMessageBox::SetText(const wchar_t* text)
{
	static_cast<CUIText*>(m_Children[0])->SetText(text);
}
