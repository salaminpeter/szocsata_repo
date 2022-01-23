#pragma once

#include "UILayout.h"

class CUIVerticalLayout : public CUILayout
{
public:

	friend CUIVerticalLayout;

	CUIVerticalLayout(bool isVertical, float x, float y, float w, float h, float whRatio, int minGap, int maxGap, int maxW, int maxH, float topPercent, float leftPercent, int elemCount, int vx, int vy, CUIElement* parent, const wchar_t* id) :
		CUILayout(x, y, w, h, vx, vy, parent, id),
		m_TopGapPercent(topPercent),
		m_LeftGapPercent(leftPercent),
		m_IsVertical(isVertical)
	{
		m_LayoutBoxes.reserve(elemCount);
		m_LayoutBoxes.insert(m_LayoutBoxes.end(), elemCount, TLayoutBox(0, 0, minGap, maxGap, maxW, maxH, whRatio, true, m_IsVertical ? CUIElement::Center : CUIElement::None, m_IsVertical ? CUIElement::None : CUIElement::Center));

		//dummy elem for storing last gap
		m_LayoutBoxes.insert(m_LayoutBoxes.end(), 1, TLayoutBox(0, 0, minGap, maxGap, 0, 0, 0, true, CUIElement::None, CUIElement::None));
		SetPosition(x, y, false);
	}
	
	virtual void AlignChildren() override;

protected:

	virtual void PositionLayoutBoxes() override;

protected:

	bool m_IsVertical;

private:

	float m_TopGapPercent = .5f;
	float m_LeftGapPercent = .5f;
};

class CUIHorizontalLayout : public CUIVerticalLayout
{
public:

	CUIHorizontalLayout(float x, float y, float w, float h, float whRatio, int minGap, int maxGap, int maxW, int maxH, float topPercent, float leftPercent, int elemCount, int vx, int vy, CUIElement* parent, const wchar_t* id) :
		CUIVerticalLayout(false, x, y, w, h, whRatio, minGap, maxGap, maxW, maxH, topPercent, leftPercent, elemCount, vx, vy, parent, id)
	{
	}

};