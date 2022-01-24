#include "stdafx.h"
#include "UIVerticalLayout.h"


void CUIVerticalLayout::AlignChildren()
{
	int Gap = 0;

	int MinGap = GetGapSum(true);
	int MaxGap = GetGapSum(false);

	while (true)
	{
		Gap = m_IsVertical ? m_Height - GetHeightSum() : m_Width - GetWidthSum();

		if (Gap > MaxGap)
			Gap = MaxGap;

		bool Success = true;
				
		for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		{
			if (i != m_LayoutBoxes.size() - 1 && (m_LayoutBoxes[i].m_Height > m_LayoutBoxes[i].m_MaxHeight || m_LayoutBoxes[i].m_Width > m_LayoutBoxes[i].m_MaxWidth || Gap < MinGap))
			{
				m_LayoutBoxes[i].m_Width--;
				m_LayoutBoxes[i].m_Height = float(m_LayoutBoxes[i].m_Width) / m_LayoutBoxes[i].m_WHRatio;
				Success = false;

				//hiba tortent ne legyen vegtelen loop...
				if (m_LayoutBoxes[i].m_Width < 0 || m_LayoutBoxes[i].m_Height < 0)
					return;
			}

			m_LayoutBoxes[i].m_Gap = (MaxGap == 0 ? Gap : float(Gap) * (float(m_LayoutBoxes[i].m_MaxGap) / float(MaxGap)));
		}

		if (Success)
			break;
	}

	PositionLayoutBoxes();
	LayoutChildren();
}

void CUIVerticalLayout::PositionLayoutBoxes()
{
	float BoxesHeight = m_IsVertical ? GetHeightSum() + GetGapSum() : GetMaxHeight();
	float BoxesWidth = m_IsVertical ? GetMaxWidth() : GetWidthSum() + GetGapSum();

	int YPos = m_Height - (m_Height - BoxesHeight) * m_TopGapPercent;
	int XPos = (m_Width - BoxesWidth) * m_LeftGapPercent;

	for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
	{
		if (m_IsVertical)
		{
			YPos -= m_LayoutBoxes[i].m_Gap + m_LayoutBoxes[i].m_Height;
			XPos = (m_Width - m_LayoutBoxes[i].m_Width) * m_LeftGapPercent;
			m_LayoutBoxes[i].m_BottomLeftX = XPos;
		}
		else
		{
			m_LayoutBoxes[i].m_BottomLeftX = XPos;
			YPos = (m_Height - m_LayoutBoxes[i].m_Height) * m_TopGapPercent;
			XPos += m_LayoutBoxes[i].m_Gap + m_LayoutBoxes[i].m_Width;
		}

		m_LayoutBoxes[i].m_BottomLeftY = YPos;
	}
}
