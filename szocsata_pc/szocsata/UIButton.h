#pragma once

#include "UIElement.h"

class CSquarePositionData;
class CSquareColorData;


class CUIButton : public CUIElement
{
public:

	CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, bool ignoreReleaseEvent = true);

	void AddText(const wchar_t* buttonText, float relTextHeight, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> gridcolorData8x8);
	void CenterText();
	void SetText(const wchar_t* buttonText);

	void Render(CRenderer* renderer) override;
	bool HandleEventAtPos(int x, int y, bool touchEvent) override;

private:

	float m_RelTextSize;
	std::wstring m_Text;
};
