#pragma once

#include "UIElement.h"

class CSquarePositionData;
class CSquareColorData;


class CUIButton : public CUIElement
{
public:
	
	enum EAlignment {Center, Left, Right};

	CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, const char* shaderID = "textured");

	void AddText(const wchar_t* buttonText, float relTextHeight, EAlignment textAlignment, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> gridcolorData8x8, float padding = 5.f);
	void PositionText();
	void SetText(const wchar_t* buttonText);

	void Render(CRenderer* renderer) override;

private:

	float m_RelTextSize;
	float m_Padding; //only used in case of Left/Right alignment
	std::wstring m_Text;
	EAlignment m_TextAlignment = Center;
};
