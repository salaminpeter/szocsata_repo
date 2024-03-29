#pragma once

#include "UIElement.h"

#include <vector>

class CSquarePositionData;
class CSquareColorData;
class CUIButton;
class CUIText;

class CUIPanel : public CUIElement
{
public:
	
	CUIPanel(CUIElement* parent, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty);

	CUIButton* AddButton(float x, float y, float w, float h, const char* textureID, const wchar_t* id);
	CUIText* AddText(const wchar_t* text, float x, float y, float fontHeight, const char* textureID, const wchar_t* id, float r = 0.92f, float g = 0.92f, float b = 0.92f);

protected:

	std::shared_ptr<CSquarePositionData> m_PositionData;
	std::shared_ptr<CSquareColorData>	 m_ColorData;
	std::shared_ptr<CSquareColorData>    m_GridColorData;
};

