#include "stdafx.h"
#include "UIIconTextButton.h"
#include "UIButton.h"
#include "UIText.h"


CUIIconTextButton::CUIIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, bool ignoreReleaseEvent, const char* shaderID) :
	CUIButton(parent, positionData, colorData, x, y, w, h, vx, vy, textureID, id)
{
	AddText(text, 0.6f, positionData, gridcolorData8x8);
}
