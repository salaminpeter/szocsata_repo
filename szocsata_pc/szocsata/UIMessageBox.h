#pragma once

#include "UIElement.h"

class CSquarePositionData;
class CSquareColorData;
class CUIButton;
class CUIText;

class CUIMessageBox : public CUIElement
{
public:

	CUIMessageBox(const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id);

	void Render(CRenderer* renderer) override {};

private:

	std::unique_ptr<CUIButton> m_OkButton;
	std::unique_ptr<CUIText> m_Text;
};
