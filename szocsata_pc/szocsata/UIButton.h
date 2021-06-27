#pragma once

#include "UIElement.h"

class CSquarePositionData;
class CSquareColorData;


class CUIButton : public CUIElement
{
public:

	CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, bool ignoreReleaseEvent = true);

	void Render(CRenderer* renderer) override;
	bool HandleEventAtPos(int x, int y, bool touchEvent) override;
};
