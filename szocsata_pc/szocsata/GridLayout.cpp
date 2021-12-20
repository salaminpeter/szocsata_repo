#include "stdafx.h"
#include "GridLayout.h"

#include <cmath>


void CGridLayout::AllignGrid(int gridCount, bool recalcGridSize, bool useGapToEdge)
{
	m_GridPositions.clear();
	m_GridPositions.reserve(gridCount);

	if (recalcGridSize)
	{
		int RowCount = 1;
		int GridsInRow;
		int BestRowCount = 0;
		float BestSize = 0.f;
		float MaxArea = 0.f;
		bool AllignmentFound = false;

		while (true)
		{
			while (true)
			{
				if (RowCount * m_MinGridSize + (RowCount + 1) * m_MinGridGap > m_Height)
					break;

				int MaxGridOnRow = gridCount / RowCount + (gridCount % RowCount ? 1 : 0);
				int VGapCount = useGapToEdge ? (RowCount + 1) : (RowCount > 1 ? RowCount - 1 : 0);
				int HGapCount = useGapToEdge ? (MaxGridOnRow + 1) : (MaxGridOnRow > 1 ? MaxGridOnRow - 1 : 0);
				float MaxVSize = (m_Height - VGapCount * m_MinGridGap) / float(RowCount);
				float MaxHSize = (m_Width - HGapCount * m_MinGridGap) / float(MaxGridOnRow);
			
				if (MaxHSize > m_MinGridSize)
				{
					float Size = std::fmin(MaxVSize, MaxHSize);
			
					if (MaxArea < Size * Size * gridCount)
					{
						MaxArea = Size * Size * gridCount;
						BestSize = Size;
						BestRowCount = RowCount;
						AllignmentFound = true;
					}
				}

				RowCount++;
			}

			if (AllignmentFound)
				break;

			m_MinGridSize *= 0.8f;
			m_MinGridGap *= 0.8f;


		}

		m_RowCount = BestRowCount;
		m_GridSize = BestSize;
		m_GridsInRow = gridCount / m_RowCount + (gridCount % m_RowCount ? 1 : 0);
		m_GridGapHoriz = (m_Width - m_GridsInRow * m_GridSize) / (m_GridsInRow + 1.f);
		m_GridGapVert = (m_Height - m_RowCount * m_GridSize) / (m_RowCount + 1.f);
	}

	float XPos = m_XPosition + (useGapToEdge ? m_GridGapHoriz : 0);
	float YPos = m_YPosition + (useGapToEdge ? m_GridGapVert : 0);
	float LastRowGrids = gridCount % m_GridsInRow;
	float LastRowOffset = (m_Width - (LastRowGrids  * m_GridSize + (LastRowGrids + 1) * m_GridGapHoriz)) / 2.f;

	for (int i = 0; i < gridCount; ++i)
	{
		if (i != 0 && (i % m_GridsInRow) == 0)
		{ 
			XPos = m_XPosition + m_GridGapHoriz;
			YPos += m_GridGapVert + m_GridSize;
		}

		float RowOffset = 0.f;

		if (gridCount % m_GridsInRow &&  gridCount - i < m_GridsInRow)
			RowOffset = LastRowOffset;

		m_GridPositions.emplace_back(XPos + RowOffset, XPos + RowOffset + m_GridSize, YPos, YPos + m_GridSize);

		XPos += m_GridGapHoriz + m_GridSize;
	}
}

int CGridLayout::GetGridIdxAtPos(int x, int y)
{
	for (size_t i = 0; i < m_GridPositions.size(); ++i)
	{
		if (x > m_GridPositions[i].m_Left && x < m_GridPositions[i].m_Right && y < m_GridPositions[i].m_Bottom && y > m_GridPositions[i].m_Top)
			return i;
	}

	return -1;
}

float CGridLayout::GetElemSize()
{
	if (!m_GridPositions.size())
		return -1;

	return m_GridPositions[0].m_Right - m_GridPositions[0].m_Left;
}
