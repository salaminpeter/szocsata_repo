#include "stdafx.h"
#include "UIButton.h"
#include "UIText.h"
#include "SquareModelData.h"
#include "Model.h"
#include "Renderer.h"


CUIButton::CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, bool ignoreReleaseEvent, const char* shaderID) :
	CUIElement(parent, id, new CModel(false, 2, positionData.get(), colorData.get(), textureID, shaderID), x, y, w, h, vx, vy, 0.f, 0.f, ignoreReleaseEvent)
{
	PositionElement();
}

void CUIButton::AddText(const wchar_t* buttonText, float relTextHeight, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> gridcolorData8x8)
{
	m_Text = buttonText;
	m_RelTextSize = relTextHeight;
	new CUIText(this, positionData, gridcolorData8x8, buttonText, 0, 0, relTextHeight * m_Height, relTextHeight * m_Height, m_ViewXPosition, m_ViewYPosition, L"ui_button_text");
	CenterText();
}

void CUIButton::SetText(const wchar_t* buttonText)
{
	CUIText* ButtonText = static_cast<CUIText*>(GetChild(L"ui_button_text"));

	if (!ButtonText)
		return;

	m_Text = buttonText;
	ButtonText->SetText(buttonText);
	CenterText();
}

void CUIButton::CenterText()
{
	CUIText* ButtonText = static_cast<CUIText*>(GetChild(L"ui_button_text"));

	if (!ButtonText)
		return;

	float CharHeight = m_RelTextSize * m_Height;
	float TextWidth = CUIText::GetTextWidthInPixels(m_Text, CharHeight);
	glm::vec2 TextTopBottom = CUIText::GetTextTopBottom(m_Text, CharHeight);
	ButtonText->SetPosAndSize((CharHeight - TextWidth) / 2.f, (CharHeight - TextTopBottom.x) / 2.f, CharHeight, CharHeight);
}

bool CUIButton::HandleEventAtPos(int x, int y, bool touchEvent)
{
	if ((touchEvent || !m_IgnoreReleaseEvent) && m_Visible && m_Enabled && PositionInElement(x, y))
	{
		HandleEvent();
		return true;
	}

	return false;
}


void CUIButton::Render(CRenderer* renderer)
{
	if (!m_Visible)
		return;

	bool Transparent = false;
	//TODO ocsmany igy! egy texture shader legyen kivulrol allithato alfaval!
	if (std::wstring(m_ID) == L"ui_dragged_player_letter_btn")
		Transparent = true;
	
	renderer->SetTexturePos(m_TexturePosition, Transparent);
	renderer->DrawModel(m_Model, "view_ortho", m_Model->GetShaderId(), false);

	//draw button text
	if (m_Children.size())
		m_Children[0]->Render(renderer);
}
