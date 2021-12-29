#pragma once

#include <vector>
#include "UIPanel.h"

class CUIElement;
class CSquarePositionData;
class CSquareColorData;
class CRenderer;

class CUISelectControl : public CUIPanel
{
public:

	CUISelectControl(CUIElement* parent, const wchar_t* id, const wchar_t* description, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, size_t idx = 0);

	void AddOption(const wchar_t* text, bool setText = false);
	void SetIndex(size_t idx);
	void SetTextAndPos(const wchar_t* text);

	int GetIndex() {return m_CurrSelection;}

private:

	void ChangeEvent(int dir);

private:

	std::vector<std::wstring> m_Options;
	size_t m_CurrSelection = 0;
};
