#include "stdafx.h"
#include "UIButton.h"
#include "UIText.h"
#include "SquareModelData.h"
#include "Model.h"
#include "Renderer.h"


CUIButton::CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, const char* shaderID) :
	CUIElement(parent, id, new CModel(false, 2, std::static_pointer_cast<CModelPositionData>(positionData), std::static_pointer_cast<CModelColorData>(colorData), textureID, shaderID), x, y, w, h, vx, vy, 0.f, 0.f)
{
	PositionElement();
}

void CUIButton::AddText(const wchar_t* buttonText, float relTextHeight, CUIText::ETextAlign textAlignment, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float padding)
{
	m_Text = buttonText;
	m_RelTextSize = relTextHeight;
	m_Padding = padding;
	m_TextAlignment = textAlignment;
	new CUIText(this, positionData, gridcolorData8x8, buttonText, relTextHeight * m_Height, 0, 0, m_ViewXPosition, m_ViewYPosition, 1, 1, 1, L"ui_button_text");
	PositionText();
}

void CUIButton::SetText(const wchar_t* buttonText)
{
	CUIText* ButtonText = static_cast<CUIText*>(GetChild(L"ui_button_text"));

	if (!ButtonText)
		return;

	m_Text = buttonText;
	ButtonText->SetText(buttonText);
	PositionText();
}

void CUIButton::PositionText()
{
	CUIText* ButtonText = static_cast<CUIText*>(GetChild(L"ui_button_text"));

	if (!ButtonText)
		return;

	ButtonText->Align(m_TextAlignment, m_Padding);
}

