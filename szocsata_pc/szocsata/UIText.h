#pragma once

#include "UIElement.h"

#include <glm/glm.hpp>
#include <map>
#include <algorithm>

class CSquarePositionData;
class CSquareColorData;
class CModel;

class CUIText : public CUIElement
{
public:

	CUIText(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const wchar_t* text, int x, int y, int w, int h, int vx, int vy, float r, float g, float b,  const wchar_t* id);

	void SetText(const wchar_t* text);
	size_t Length() const;
	void SetColor(float r, float g, float b);

	void Render(CRenderer* renderer) override;

	static float GetTextWidthInPixels(const std::wstring& text, int size);
	static float GetTextSize(const std::wstring& text, int textWidth);
	static glm::vec2 GetTextTopBottom(const std::wstring& text, int size);
	static void InitFontTexPositions();

private:

	std::wstring m_Text;

	glm::vec3 m_Color;

	static std::map<wchar_t, glm::vec2> m_FontTexPos; //TODO kulon font osztaly!!!!!!!!!!
	static std::map<wchar_t, int> m_FontCharWidth;
	static std::map<wchar_t, int> m_FontCharHeight;
	static std::map<wchar_t, float> m_FontDesc;
};