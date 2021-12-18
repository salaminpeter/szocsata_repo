#pragma once

#include <vector>
#include "UIButton.h"

class CUIElement;
class CSquarePositionData;
class CSquareColorData;
class CRenderer;

class CUIIconTextButton : public CUIButton
{
public:

	CUIIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float x, float y, float w, float h, float vx, float vy, const char* textureID, const char* iconTextureID, const wchar_t* id, const char* shaderID = "textured", float iconSize = 0.65f, float charSize = 0.35f, float padding = 0.f, CUIText::ETextAlign align = CUIText::Left);

	void SetIconColor(float r, float g, float b);
	void CenterIcon();
};
