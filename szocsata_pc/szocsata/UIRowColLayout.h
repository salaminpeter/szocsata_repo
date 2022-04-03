#pragma once

#include "UILayout.h"

class CUIRowColLayout : public CUILayout
{
public:

	CUIRowColLayout(bool isVertical, float x, float y, float w, float h, int vx, int vy, CUIElement* parent, const wchar_t* id, int elemCount, float topPercent, float leftPercent, float whRatio = 1.f, int minGap = 0.f, int maxGap = 0.f, int maxW = 0.f, int maxH = 0.f) :
		CUILayout(x, y, w, h, vx, vy, parent, id, elemCount, whRatio, minGap, maxGap, maxW, maxH),
		m_TopGapPercent(topPercent),
		m_LeftGapPercent(leftPercent),
		m_IsVertical(isVertical)
	{
		AddLayoutBoxes(elemCount,whRatio, minGap, maxGap, maxW, maxH);

		//dummy elem for storing last gap
		m_LayoutBoxes.insert(m_LayoutBoxes.end(), 1, TLayoutBox(0, 0, minGap, maxGap, 0, 0, 0, true));

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


class CUIVerticalLayout : public CUIRowColLayout
{
public:

	CUIVerticalLayout(float x, float y, float w, float h, int vx, int vy, CUIElement* parent, const wchar_t* id, int elemCount, float topPercent, float leftPercent, float whRatio = 1.f, int minGap = 0.f, int maxGap = 0.f, int maxW = 0.f, int maxH = 0.f) :
		CUIRowColLayout(true, x, y, w, h, vx, vy, parent, id, elemCount, topPercent, leftPercent, whRatio, minGap, maxGap, maxW, maxH)
	{
	}
};

class CUIHorizontalLayout : public CUIRowColLayout
{
public:

	CUIHorizontalLayout(float x, float y, float w, float h, int vx, int vy, CUIElement* parent, const wchar_t* id, int elemCount, float topPercent, float leftPercent, float whRatio = 1.f, int minGap = 0.f, int maxGap = 0.f, int maxW = 0.f, int maxH = 0.f) :
		CUIRowColLayout(false, x, y, w, h, vx, vy, parent, id, elemCount, topPercent, leftPercent, whRatio, minGap, maxGap, maxW, maxH)
	{
	}
};