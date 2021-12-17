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

	enum ETextAlign {Left, Right, Center};

	CUIText(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const wchar_t* text, int fontHeight, int x, int y, int vx, int vy, float r, float g, float b,  const wchar_t* id);

	void SetText(const wchar_t* text);
	size_t Length() const;
	void SetColor(float r, float g, float b);
	void Align(ETextAlign alingment, float padding = 0.f);

	static float GetTextWidthInPixels(const std::wstring& text, int size);
	static float GetTextHeightInPixels(const std::wstring& text, int size);
	static float GetTextSize(const std::wstring& text, int textWidth);
	static glm::vec2 GetTextTopBottom(const std::wstring& text, int size);
	static void InitFontTexPositions();

	static float m_FontTextureCharWidth;
	static float m_FontTextureCharHeight;

private:

	std::wstring m_Text;
	glm::vec3 m_Color;
	float m_FontHeight;

	static std::map<wchar_t, glm::vec2> m_FontTexPos; //TODO kulon font osztaly!!!!!!!!!!
	static std::map<wchar_t, int> m_FontCharWidth;
	static std::map<wchar_t, int> m_FontCharHeight;
	static std::map<wchar_t, float> m_FontDesc;
};