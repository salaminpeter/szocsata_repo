#include "stdafx.h"
#include "UILayout.h"

void CUILayout::LayoutChildren()
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		if ((m_LayoutBoxes[i].m_IncSizeAllowed && (m_Children[i]->GetWidth() < m_LayoutBoxes[i].m_Width || m_Children[i]->GetHeight() < m_LayoutBoxes[i].m_Height)) 
											   || (m_Children[i]->GetWidth() > m_LayoutBoxes[i].m_Width || m_Children[i]->GetHeight() > m_LayoutBoxes[i].m_Height))
			m_Children[i]->Resize(m_LayoutBoxes[i].m_Width, true);

		if (m_LayoutBoxes[i].m_IncSizeAllowed)
			m_Children[i]->SetPosition(m_LayoutBoxes[i].m_BottomLeftX, m_LayoutBoxes[i].m_BottomLeftY, false);
		else
		{
			float ElemPosX;
			float ElemPosY;

			//align center
			if (m_LayoutBoxes[i].m_Alignment == EAlignmentType::Center)
			{
				ElemPosX = m_LayoutBoxes[i].m_BottomLeftX + m_LayoutBoxes[i].m_Width / 2 - m_Children[i]->GetWidth() / 2;
				ElemPosY = m_LayoutBoxes[i].m_BottomLeftY + m_LayoutBoxes[i].m_Height / 2 - m_Children[i]->GetHeight() / 2;
				m_Children[i]->SetPosition(ElemPosX, ElemPosY, false);
			}

			//align left
			else if (m_LayoutBoxes[i].m_Alignment == EAlignmentType::Left)
			{
				ElemPosX = m_LayoutBoxes[i].m_BottomLeftX;
				ElemPosY = m_LayoutBoxes[i].m_BottomLeftY + m_LayoutBoxes[i].m_Height / 2 - m_Children[i]->GetHeight() / 2;
			}

			//align right
			else if (m_LayoutBoxes[i].m_Alignment == EAlignmentType::Right)
			{
				ElemPosX = m_LayoutBoxes[i].m_BottomLeftX + m_LayoutBoxes[i].m_Width - m_Children[i]->GetWidth();
				ElemPosY = m_LayoutBoxes[i].m_BottomLeftY + m_LayoutBoxes[i].m_Height / 2 - m_Children[i]->GetHeight() / 2;
			}
		}

		m_Children[i]->PositionElement();
	}
}

void CUILayout::SetBoxSizeProps(size_t idx, int maxWidth, int maxHeight)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_MaxWidth = maxWidth;
	m_LayoutBoxes[idx].m_MaxHeight = maxHeight;
	m_LayoutBoxes[idx].m_Width = maxWidth;
	m_LayoutBoxes[idx].m_Height = maxHeight;
}

void CUILayout::SetBoxGapProps(size_t idx, int minGap, int maxGap)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_MinGap = minGap;
	m_LayoutBoxes[idx].m_MaxGap = maxGap;
}

void CUILayout::SetBoxAlignProps(size_t idx, int alignment, bool incSize)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_Alignment = alignment;
	m_LayoutBoxes[idx].m_IncSizeAllowed = incSize;
}

float CUILayout::GetGapSum(bool min)
{
	float GapSum = 0;

	GapSum += min ? m_LayoutBoxes[0].m_MinGap : m_LayoutBoxes[0].m_MaxGap;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		GapSum += min ? m_LayoutBoxes[i].m_MinGap : m_LayoutBoxes[i].m_MaxGap;

	return GapSum;
}

float CUILayout::GetGapSum()
{
	float GapSum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
		GapSum += m_LayoutBoxes[i].m_Gap;

	return GapSum;
}

float CUILayout::GetHeightGapSum()
{
	float Sum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		Sum += m_LayoutBoxes[i].m_Height + (i != m_LayoutBoxes.size() - 1 ? m_LayoutBoxes[i].m_Gap : 0);

	return Sum;
}

float CUILayout::GetHeightSum()
{
	float HeightSum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		HeightSum += m_LayoutBoxes[i].m_Height;

	return HeightSum;
}

float CUILayout::GetMaxWidth()
{
	float MaxWidth = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		if (m_LayoutBoxes[i].m_Width > MaxWidth)
			MaxWidth = m_LayoutBoxes[i].m_Width;

	return MaxWidth;
}

