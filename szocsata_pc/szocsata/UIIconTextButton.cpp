#include "stdafx.h"
#include "UIIconTextButton.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIPanel.h"


CUIIconTextButton::CUIIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float x, float y, float w, float h, float vx, float vy, const char* textureID, const char* iconTextureID, const wchar_t* id, const char* shaderID, float iconSize, float charSize, float padding, CUIText::ETextAlign align) :
	CUIButton(parent, positionData, colorData, x, y, w, h, vx, vy, textureID, id)
{
	m_CheckChildEvents = false;

	float IconSize = h * iconSize;
	float IconPadding = std::fabs(padding) < 0.001f ? IconSize / 5 : padding;

	AddText(text, charSize, align, positionData, gridcolorData8x8, IconSize + 2 * IconPadding);
	CUIPanel* IconPanel = new CUIPanel(this, L"ui_button_icon", positionData, colorData, gridcolorData8x8, -w / 2 + IconSize / 2 + IconPadding, 0, IconSize, IconSize, 0, 0, iconTextureID, 0, 0);
}

void CUIIconTextButton::SetTextColor(float r, float g, float b)
{
	static_cast<CUIText*>(GetChild(L"ui_button_text"))->SetColor(r, g, b);
}

void CUIIconTextButton::SetIconColor(float r, float g, float b)
{
	GetChild(L"ui_button_icon")->SetModifyColor(glm::vec4(r, g, b, 1));
}

void CUIIconTextButton::CenterIcon()
{
	GetChild(L"ui_button_icon")->SetPosition(0.f, 0.f);
}

