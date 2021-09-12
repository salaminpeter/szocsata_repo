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

	CUIIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float x, float y, float w, float h, float vx, float vy, const char* textureID, const char* iconTextureID, const wchar_t* id, bool ignoreReleaseEvent = true, const char* shaderID = "textured");
};
