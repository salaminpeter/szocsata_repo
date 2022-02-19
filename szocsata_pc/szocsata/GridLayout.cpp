#include "stdafx.h"
#include "GridLayout.h"

#include <cmath>


void CGridLayout::AlignChildren()
{
	int ElemCount = m_LayoutBoxes.size();
	int RowCount = 1;
	int BestRowCount = 0;
	float MinGridSize = m_Height;
	float MinGridGap = MinGridSize / 4;
	float GridSize = 0.f;
	float MaxArea = 0.f;
	bool AllignmentFound = false;

	//calculate best row count, grid size, and gaps
	while (true)
	{
		while (true)
		{
			if (RowCount * MinGridSize + (RowCount + 1) * MinGridGap > m_Height)
				break;

			int MaxGridOnRow = ElemCount / RowCount + (ElemCount % RowCount ? 1 : 0);
			int VGapCount = RowCount > 1 ? RowCount - 1 : 0;
			int HGapCount = MaxGridOnRow > 1 ? MaxGridOnRow - 1 : 0;
			float MaxVSize = (m_Height - VGapCount * MinGridGap) / float(RowCount);
			float MaxHSize = (m_Width - HGapCount * MinGridGap) / float(MaxGridOnRow);

			if (MaxHSize > MinGridSize)
			{
				float Size = std::fmin(MaxVSize, MaxHSize);

				if (MaxArea < Size * Size * ElemCount)
				{
					MaxArea = Size * Size * ElemCount;
					GridSize = Size;
					BestRowCount = RowCount;
					AllignmentFound = true;
				}
			}

			RowCount++;
		}

		if (AllignmentFound)
			break;

		MinGridSize *= 0.9f;
		MinGridGap *= 0.9f;
	}

	int GridsInRow = ElemCount / BestRowCount + (ElemCount % BestRowCount ? 1 : 0);
	float GridGapHoriz = (m_Width - GridsInRow * GridSize) / (GridsInRow + 1.f);
	float GridGapVert = (m_Height - BestRowCount * GridSize) / (BestRowCount + 1.f);

	//position layout boxes
	float XPos = GridGapHoriz;
	float YPos = GridGapVert;
	float LastRowGrids = ElemCount % GridsInRow;
	float LastRowOffset = (m_Width - (LastRowGrids  * GridSize + (LastRowGrids + 1) * GridGapHoriz)) / 2.f;

	for (int i = 0; i < ElemCount; ++i)
	{
		if (i != 0 && (i % GridsInRow) == 0)
		{
			XPos = GridGapHoriz;
			YPos += GridGapVert + GridSize;
		}

		float RowOffset = 0.f;

		if (ElemCount % GridsInRow &&  ElemCount - i < GridsInRow)
			RowOffset = LastRowOffset;

		m_LayoutBoxes[i].m_BottomLeftX = XPos + RowOffset;
		m_LayoutBoxes[i].m_BottomLeftY = YPos;
		m_LayoutBoxes[i].m_Width = GridSize;
		m_LayoutBoxes[i].m_Height = GridSize;

		XPos += GridGapHoriz + GridSize;
	}

	LayoutChildren();
}

int CGridLayout::GetGridIdxAtPos(int x, int y)
{
	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
	{
		float GridSize = m_LayoutBoxes[i].m_Width;
		
		if (x > m_LayoutBoxes[i].m_BottomLeftX && x < m_LayoutBoxes[i].m_BottomLeftX + GridSize && y < m_LayoutBoxes[i].m_BottomLeftY + GridSize && y > m_LayoutBoxes[i].m_BottomLeftY)
			return i;
	}

	return -1;
}

float CGridLayout::GetElemSize()
{
	if (m_LayoutBoxes.size() == 0)
		return -1;

	return m_LayoutBoxes[0].m_Width;
}
