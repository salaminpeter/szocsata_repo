#include "stdafx.h"
#include "UIMessageBox.h"
#include "UIText.h"
#include "UIIconTextButton.h"

CUIMessageBox* CUIMessageBox::m_ActiveMessageBox = nullptr;
int CUIMessageBox::m_RetValue = false;


CUIMessageBox::CUIMessageBox(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type) :
	CUIPanel(nullptr, L"ui_message_box", positionData, colorData, gridColorData, x, y, w, h, vx, vy, "messagebox_texture_generated", 0.f, 0.f),
	m_Type(type)
{
	float TextPanelYPos = m_Type == NoButton ? 0.f : h / 2 - h / 6;
	CUIPanel* TextPanel = new CUIPanel(this, L"ui_msgbox_text_panel", positionData, colorData, gridColorData, 0, TextPanelYPos, w, h / 3, vx, vy, "", 0.f, 0.f);
	
	m_TextSize = h / 3 - h / 10;
	TextPanel->AddText(L"", 0.f, 0.f, m_TextSize, "font.bmp", L"ui_message_box_text");

	if (m_Type == Ok)
	{ 
		float BtnSize = m_TextSize * 1.5f;
		CUIIconTextButton* NewButton = new CUIIconTextButton(this, L"", positionData, colorData, nullptr, 0, -h / 2 + BtnSize / 2 + BtnSize / 4, BtnSize, BtnSize, 0, 0, "round_button_texture_generated", "ok_icon.bmp", L"msg_box_ok_button");
		NewButton->SetEvent(false, this, &CUIMessageBox::ButtonPressed, 1);
	}

	if (m_Type == OkCancel)
	{
		AddButton(80, -80, 50, 50, "cancelbutton.bmp", L"ui_message_box_cancel_btn");
		m_Children.back()->SetEvent(false, this, &CUIMessageBox::ButtonPressed, 0);
	}
}

void CUIMessageBox::ButtonPressed(int ret)
{
	m_RetValue = ret;
	m_ActiveMessageBox = nullptr;
}

void CUIMessageBox::SetText(const wchar_t* text)
{
	
	CUIText* Text = static_cast<CUIText*>(GetChild(L"ui_message_box_text"));
	Text->SetText(text);
	Text->Align(CUIText::Center);
}
