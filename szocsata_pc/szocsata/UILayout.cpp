#include "stdafx.h"
#include "UILayout.h"

void CUILayout::LayoutChildren()
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		m_Children[i]->AlignChildren();

		if ((m_LayoutBoxes[i].m_IncSizeAllowed && (m_Children[i]->GetWidth() < m_LayoutBoxes[i].m_Width || m_Children[i]->GetHeight() < m_LayoutBoxes[i].m_Height)) 
											   || (m_Children[i]->GetWidth() > m_LayoutBoxes[i].m_Width || m_Children[i]->GetHeight() > m_LayoutBoxes[i].m_Height))
		{
			float PrevWidth = m_Children[i]->GetWidth();
			float PrevHeight = m_Children[i]->GetHeight();

			if (m_LayoutBoxes[i].m_Width > m_LayoutBoxes[i].m_Height)
				m_Children[i]->Resize(m_LayoutBoxes[i].m_Width, true);
			else
				m_Children[i]->Resize(m_LayoutBoxes[i].m_Height, false);
		}

		m_Children[i]->SetPosition(m_LayoutBoxes[i].m_BottomLeftX, m_LayoutBoxes[i].m_BottomLeftY, false);

		glm::vec2 ElemPos = m_Children[i]->GetPosition(false);

		//horizontal alignment
		//align center
		if (m_LayoutBoxes[i].m_AlignmentH == EAlignmentType::Center)
			ElemPos.x = m_LayoutBoxes[i].m_BottomLeftX + m_LayoutBoxes[i].m_Width / 2 - m_Children[i]->GetWidth() / 2;

		//align left
		else if (m_LayoutBoxes[i].m_AlignmentH == EAlignmentType::Left)
			ElemPos.x = m_LayoutBoxes[i].m_BottomLeftX;

		//align right
		else if (m_LayoutBoxes[i].m_AlignmentH == EAlignmentType::Right)
			ElemPos.x = m_LayoutBoxes[i].m_BottomLeftX + m_LayoutBoxes[i].m_Width - m_Children[i]->GetWidth();

		//vertical alignment
		//align center
		if (m_LayoutBoxes[i].m_AlignmentV == EAlignmentType::Center)
			ElemPos.y = m_LayoutBoxes[i].m_BottomLeftY + m_LayoutBoxes[i].m_Height / 2 - m_Children[i]->GetHeight() / 2;

		//align top
		else if (m_LayoutBoxes[i].m_AlignmentV == EAlignmentType::Top)
			ElemPos.y = m_LayoutBoxes[i].m_BottomLeftY + m_LayoutBoxes[i].m_Height - m_Children[i]->GetHeight();

		//align bottom
		else if (m_LayoutBoxes[i].m_AlignmentV == EAlignmentType::Bottom)
			ElemPos.y = m_LayoutBoxes[i].m_BottomLeftY;

		m_Children[i]->SetPosition(ElemPos.x, ElemPos.y, false);
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
	m_LayoutBoxes[idx].m_WHRatio = float(maxWidth) / float(maxHeight);
}

void CUILayout::SetBoxGapProps(size_t idx, int minGap, int maxGap)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_MinGap = minGap;
	m_LayoutBoxes[idx].m_MaxGap = maxGap;
}

void CUILayout::SetBoxAlignProps(size_t idx, int alignmentH, int alignmentV, bool incSize)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_AlignmentH = alignmentH;
	m_LayoutBoxes[idx].m_AlignmentV = alignmentV;
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

float CUILayout::GetWidthSum()
{
	float WidthSum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		WidthSum += m_LayoutBoxes[i].m_Width;

	return WidthSum;
}

float CUILayout::GetMaxHeight()
{
	float MaxHeight = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		if (m_LayoutBoxes[i].m_Height > MaxHeight)
			MaxHeight = m_LayoutBoxes[i].m_Width;

	return MaxHeight;
}

float CUILayout::GetMaxWidth()
{
	float MaxWidth = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		if (m_LayoutBoxes[i].m_Width > MaxWidth)
			MaxWidth = m_LayoutBoxes[i].m_Width;

	return MaxWidth;
}

