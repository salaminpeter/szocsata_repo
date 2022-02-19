#include "stdafx.h"
#include "UIRowColLayout.h"


void CUIRowColLayout::AlignChildren()
{
	int Gap = 0;

	int MinGap = GetGapSum(true);
	int MaxGap = GetGapSum(false);

	float HeightSum = GetHeightSum();
	float WidthSum = GetWidthSum();

	//ha a layoutboxok nem fernek ki egymas alatt mindegyiknek csokkentjuk a meretet
	if (m_IsVertical && m_Height < HeightSum)
	{
		float dif = (HeightSum - m_Height);
		  
		for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
			m_LayoutBoxes[i].m_Height -= (m_LayoutBoxes[i].m_Height / HeightSum) * dif;
	}

	else if (!m_IsVertical && m_Width < WidthSum)
	{
		float dif = (WidthSum - m_Width);

		for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
			m_LayoutBoxes[i].m_Width -= (m_LayoutBoxes[i].m_Width / WidthSum) * dif;
	}

	while (true)
	{
		Gap = m_IsVertical ? m_Height - GetHeightSum() : m_Width - GetWidthSum();

		if (Gap > MaxGap)
			Gap = MaxGap;

		if (!m_IsVertical && GetWidthSum() + GetGapSum(true) > m_Width || m_IsVertical && GetHeightSum() + GetGapSum(true) > m_Height)
		{
			int ChangeCount = m_LayoutBoxes.size();

			for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
			{ 
				if (m_LayoutBoxes[i].m_MinGap > 1)
				{
					m_LayoutBoxes[i].m_MinGap -= 1;
					
					if (m_LayoutBoxes[i].m_MinGap > m_LayoutBoxes[i].m_MaxGap)
						m_LayoutBoxes[i].m_MaxGap = m_LayoutBoxes[i].m_MinGap;
				}
				else if (m_IsVertical && m_LayoutBoxes[i].m_Height > 0)
				{
					m_LayoutBoxes[i].m_Height--;
					m_LayoutBoxes[i].m_Width = m_LayoutBoxes[i].m_WHRatio * m_LayoutBoxes[i].m_Height;
				}
				else if (!m_IsVertical && m_LayoutBoxes[i].m_Width > 0)
				{
					m_LayoutBoxes[i].m_Width--;
					m_LayoutBoxes[i].m_Height = m_LayoutBoxes[i].m_Width / m_LayoutBoxes[i].m_WHRatio;
				}
				else
					ChangeCount--;
			}

			//hiba tortent! a layoutok nem fernek ki! egyiknek sem lehetett valtoztatni a tulajdonsagait
			if (ChangeCount == 0)
				return;

			continue;
		}

		bool Success = true;
				
		MinGap = GetGapSum(true);
		MaxGap = GetGapSum(false);

		for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		{
			if (i != m_LayoutBoxes.size() - 1 && (m_LayoutBoxes[i].m_Height > m_LayoutBoxes[i].m_MaxHeight || m_LayoutBoxes[i].m_Width > m_LayoutBoxes[i].m_MaxWidth || Gap < MinGap))
			{
				m_LayoutBoxes[i].m_Width--;
				m_LayoutBoxes[i].m_Height = float(m_LayoutBoxes[i].m_Width) / m_LayoutBoxes[i].m_WHRatio;
				Success = false;

				//hiba tortent ne legyen vegtelen loop...
				if (m_LayoutBoxes[i].m_Width < 1 || m_LayoutBoxes[i].m_Height < 1)
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

void CUIRowColLayout::PositionLayoutBoxes()
{
	float BoxesHeight = m_IsVertical ? GetHeightSum() + GetGapSum() : GetMaxHeight();
	float BoxesWidth = m_IsVertical ? GetMaxWidth() : GetWidthSum() + GetGapSum();

	int YPos = m_Height - (m_Height - BoxesHeight) * m_TopGapPercent;
	int XPos = (m_Width - BoxesWidth) * m_LeftGapPercent + (!m_IsVertical ? m_LayoutBoxes[0].m_Gap : 0);

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
