#pragma once

#include "UIElement.h"
#include <map>
#include <algorithm>

class CSquarePositionData;
class CSquareColorData;
class CModel;

class CUIText : public CUIElement
{
public:

	CUIText(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const wchar_t* text, int x, int y, int w, int h, int vx, int vy, const wchar_t* id);

	void SetText(const wchar_t* text);
	size_t Length() const;

	void Render(CRenderer* renderer) override;

private:

	std::wstring m_Text;
	std::shared_ptr<CSquarePositionData> m_PositionData;
	std::shared_ptr<CSquareColorData> m_ColorData;

	std::map<wchar_t, glm::vec2> m_FontTexPos; //TODO kulon font osztaly!!!!!!!!!!
	std::map<wchar_t, int> m_FontCharWidth;

private:

	void InitFontTexPositions();
};