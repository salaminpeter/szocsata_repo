#pragma once

#include "UILayout.h"

class CUIVerticalLayout : public CUILayout
{
public:

	CUIVerticalLayout(float x, float y, float w, float h, float whRatio, int minGap, int maxGap, int maxW, int maxH, float topPercent, float leftPercent, int elemCount, int vx, int vy, CUIElement* parent, const wchar_t* id) :
		CUILayout(x, y, w, h, vx, vy, parent, id),
		m_MinBoxGap(minGap),
		m_MaxBoxGap(maxGap),
		m_MaxBoxWidth(maxW),
		m_MaxBoxHeight(maxH),
		m_WHRatio(whRatio),
		m_TopGapPercent(topPercent),
		m_LeftGapPercent(leftPercent)
	{
		m_LayoutBoxes.reserve(elemCount);
		m_LayoutBoxes.insert(m_LayoutBoxes.end(), elemCount, TLayoutBox(0, 0, minGap, maxGap, maxW, maxH, whRatio, true));
		SetPosition(x, y, false);
	}
	
	virtual void AlignChildren() override;

private:

	int m_MinBoxGap;
	int m_MaxBoxGap;
	int m_MaxBoxWidth;
	int m_MaxBoxHeight;
	float m_WHRatio;
	float m_TopGapPercent;
	float m_LeftGapPercent;
};