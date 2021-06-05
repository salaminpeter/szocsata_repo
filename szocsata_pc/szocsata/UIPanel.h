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
	
	CUIPanel(CUIElement* parent, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty);

	void AddButton(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* textureID, const wchar_t* id);
	void AddText(const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* textureID, const wchar_t* id);
	void Render(CRenderer* renderer) override;
};

