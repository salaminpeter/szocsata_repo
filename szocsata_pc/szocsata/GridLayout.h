#pragma once

#include "UILayout.h"

#include <vector>
#include <glm/glm.hpp>


class CGridLayout : public CUILayout
{
public:

	CGridLayout(float x, float y, float w, float h, int vx, int vy, float minGap, float minSize, CUIElement* parent, const wchar_t* id, int elemCount) :
		CUILayout(x, y, w, h, vx, vy, parent, id, elemCount), 
		m_MinGridGap(minGap), 
		m_MinGridSize(minSize)
	{
		m_LayoutBoxes.reserve(elemCount);
		m_LayoutBoxes.insert(m_LayoutBoxes.end(), elemCount, TLayoutBox(0, 0, 0, 0, 0, 0, 1.f, true));
	}


	int GetGridIdxAtPos(int x, int y);
	float GetElemSize();

	virtual void AlignChildren() override;

private:

	int m_GridsInRow;
	float m_GridGapHoriz = 0.f;
	float m_GridGapVert = 0.f;
	float m_GridSize = 0.f;
	int m_RowCount;
	float m_MinGridGap;
	float m_MinGridSize;
};