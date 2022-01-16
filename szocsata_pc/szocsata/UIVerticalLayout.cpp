#include "stdafx.h"
#include "UIVerticalLayout.h"


void CUIVerticalLayout::AlignChildren()
{
	int Gap = 0;

	float MinGap = GetGapSum(true);
	float MaxGap = GetGapSum(false);

	while (true)
	{
		Gap = m_Height - GetHeightSum();

		if (Gap > MaxGap)
			Gap = MaxGap;

		bool Success = true;
				
		for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		{
			if (m_LayoutBoxes[i].m_Height > m_LayoutBoxes[i].m_MaxHeight || m_LayoutBoxes[i].m_Width > m_LayoutBoxes[i].m_MaxWidth || Gap < MinGap)
			{
				m_LayoutBoxes[i].m_Width--;
				m_LayoutBoxes[i].m_Height = m_LayoutBoxes[i].m_Width * m_LayoutBoxes[i].m_WHRatio;
				Success = false;

				//hiba tortent ne legyen vegtelen loop...
				if (m_LayoutBoxes[i].m_Width < 0 || m_LayoutBoxes[i].m_Height < 0)
					return;
			}

			m_LayoutBoxes[i].m_Gap = MinGap == 0 ? Gap : Gap * (m_LayoutBoxes[i].m_MinGap / MinGap);
		}

		if (Success)
			break;
	}

	float BoxesHeight = GetHeightSum() + GetGapSum();
	int YPos = m_Height - (m_Height - BoxesHeight) * m_TopGapPercent;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
	{
		int XPos = (m_Width - m_LayoutBoxes[i].m_Width) * m_LeftGapPercent;
		YPos -= (i == 0 ? 0 : m_LayoutBoxes[i].m_Gap) + m_LayoutBoxes[i].m_Height;

		m_LayoutBoxes[i].m_BottomLeftX = XPos;
		m_LayoutBoxes[i].m_BottomLeftY = YPos;
	}
	
	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->AlignChildren();

	LayoutChildren();
}
