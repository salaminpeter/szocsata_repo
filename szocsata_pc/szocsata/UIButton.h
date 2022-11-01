#pragma once

#include "UIElement.h"
#include "UIText.h"

class CSquarePositionData;
class CSquareColorData;


class CUIButton : public CUIElement
{
public:
	
	CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, const char* shaderID = "textured");

	void AddText(const wchar_t* buttonText, float relTextHeight, CUIText::ETextAlign textAlignment, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float padding = 5.f);
	void PositionText();
	void SetText(const wchar_t* buttonText);
	void SetTextColor(float r, float g, float b);

private:

	float m_RelTextSize;
	float m_Padding; //only used in case of Left/Right alignment
	std::wstring m_Text;
	CUIText::ETextAlign m_TextAlignment = CUIText::Center;
};
