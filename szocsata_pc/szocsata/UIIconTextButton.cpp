#include "stdafx.h"
#include "UIIconTextButton.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIPanel.h"


CUIIconTextButton::CUIIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float x, float y, float w, float h, float vx, float vy, const char* textureID, const char* iconTextureID, const wchar_t* id, bool ignoreReleaseEvent, const char* shaderID) :
	CUIButton(parent, positionData, colorData, x, y, w, h, vx, vy, textureID, id)
{
	m_CheckChildEvents = false;

	float IconSize = h * 0.65f;
	float IconPadding = IconSize / 5;

	AddText(text, 0.35f, CUIButton::Left, positionData, gridcolorData8x8, IconSize + 2 * IconPadding);
	CUIPanel* IconPanel = new CUIPanel(this, L"ui_button_icon", positionData, colorData, gridcolorData8x8, -w / 2 + IconSize / 2 + IconPadding, 0, IconSize, IconSize, 0, 0, iconTextureID, 0, 0);
}
