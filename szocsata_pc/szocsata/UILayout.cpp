#include "stdafx.h"
#include "UILayout.h"

void CUILayout::LayoutChildren()
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		CUILayout* ChildLayout = dynamic_cast<CUILayout*>(m_Children[i]);
		bool IsChildALayout = ChildLayout != nullptr;

		//layout applyed from an other layout, set only position
		if (IsChildALayout && ChildLayout->IsAdjustedToLayout())
		{
			m_Children[i]->SetPosition(m_LayoutBoxes[i].m_BottomLeftX, m_LayoutBoxes[i].m_BottomLeftY, false);
			continue;
		}

		m_Children[i]->AlignChildren();

		if ((m_LayoutBoxes[i].m_IncSizeAllowed && (m_Children[i]->GetWidth() < m_LayoutBoxes[i].m_Width || m_Children[i]->GetHeight() < m_LayoutBoxes[i].m_Height)) 
											   || (m_Children[i]->GetWidth() > m_LayoutBoxes[i].m_Width || m_Children[i]->GetHeight() > m_LayoutBoxes[i].m_Height))
		{
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
			ElemPos.y = m_Height - m_Children[i]->GetHeight();

		//align bottom
		else if (m_LayoutBoxes[i].m_AlignmentV == EAlignmentType::Bottom)
			ElemPos.y = m_LayoutBoxes[i].m_BottomLeftY;

		m_Children[i]->SetPosition(ElemPos.x, ElemPos.y, false);
	}

	m_LayoutDone = true;

	//adjust "child" layouts
	for (size_t i = 0; i < m_LayersToAdjust.size(); ++i)
		m_LayersToAdjust[i]->AdjustToLayer();

}

bool CUILayout::GetBoxProperties(size_t idx, int& x, int& y, int& w, int& h, int& alignH, int& alignV, int& minGap, int& maxGap, int& gap, int& maxW, int& maxH, bool& inc, float& whRatio)
{
	if (idx >= m_LayoutBoxes.size())
		return false;

	x = m_LayoutBoxes[idx].m_BottomLeftX;
	y = m_LayoutBoxes[idx].m_BottomLeftY;
	w = m_LayoutBoxes[idx].m_Width;
	h = m_LayoutBoxes[idx].m_Height;
	alignH = m_LayoutBoxes[idx].m_AlignmentH;
	alignV = m_LayoutBoxes[idx].m_AlignmentV;
	minGap = m_LayoutBoxes[idx].m_MinGap;
	maxGap = m_LayoutBoxes[idx].m_MaxGap;
	gap = m_LayoutBoxes[idx].m_Gap;
	maxW = m_LayoutBoxes[idx].m_MaxWidth;
	maxH = m_LayoutBoxes[idx].m_MaxHeight;
	inc = m_LayoutBoxes[idx].m_IncSizeAllowed;
	whRatio = m_LayoutBoxes[idx].m_WHRatio;
}

void CUILayout::AdjustToLayer()
{
	//if layout to be adjusted to has not been adjusted skip
	//when layout will be adjusted, it will adjust this layout
	if (!m_AdjustToLayer->IsLayoutDone())
		return;

	int x; 
	int y; 
	int w; 
	int h; 
	int alignH; 
	int alignV; 
	int minGap; 
	int maxGap; 
	int gap; 
	int maxW; 
	int maxH; 
	bool inc; 
	float whRatio;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
	{
		//ellenorizni hogy m_AdjustToLayer-ben van e eleg layoutbox i indexhez! TODO
		m_AdjustToLayer->GetBoxProperties(i, x, y, w, h, alignH, alignV, minGap, maxGap, gap, maxW, maxH, inc, whRatio);

		m_LayoutBoxes[i].m_BottomLeftX = x;
		m_LayoutBoxes[i].m_BottomLeftY = y;
		m_LayoutBoxes[i].m_Width = w;
		m_LayoutBoxes[i].m_Height = h;
		m_LayoutBoxes[i].m_AlignmentH = alignH;
		m_LayoutBoxes[i].m_AlignmentV = alignV;
		m_LayoutBoxes[i].m_MinGap = minGap;
		m_LayoutBoxes[i].m_MaxGap = maxGap;
		m_LayoutBoxes[i].m_Gap = gap;
		m_LayoutBoxes[i].m_MaxWidth = maxW;
		m_LayoutBoxes[i].m_MaxHeight = maxH;
		m_LayoutBoxes[i].m_IncSizeAllowed = inc;
		m_LayoutBoxes[i].m_WHRatio = whRatio;
	}

	PositionLayoutBoxes();
	LayoutChildren();
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

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		GapSum += min ? m_LayoutBoxes[i].m_MinGap : m_LayoutBoxes[i].m_MaxGap;

	return GapSum;
}

float CUILayout::GetGapSum()
{
	float GapSum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size(); ++i)
		GapSum += m_LayoutBoxes[i].m_Gap;

	return GapSum;
}

float CUILayout::GetHeightSum()
{
	float HeightSum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
		HeightSum += m_LayoutBoxes[i].m_Height;

	return HeightSum;
}

float CUILayout::GetWidthSum()
{
	float WidthSum = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
		WidthSum += m_LayoutBoxes[i].m_Width;

	return WidthSum;
}

float CUILayout::GetMaxHeight()
{
	float MaxHeight = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
		if (m_LayoutBoxes[i].m_Height > MaxHeight)
			MaxHeight = m_LayoutBoxes[i].m_Width;

	return MaxHeight;
}

float CUILayout::GetMaxWidth()
{
	float MaxWidth = 0;

	for (size_t i = 0; i < m_LayoutBoxes.size() - 1; ++i)
		if (m_LayoutBoxes[i].m_Width > MaxWidth)
			MaxWidth = m_LayoutBoxes[i].m_Width;

	return MaxWidth;
}

void CUILayout::SetAdjustToLayer(CUILayout* layer)
{
	m_AdjustToLayer = layer;
	layer->AddLayerToAdjust(this);
}

void CUILayout::AddLayerToAdjust(CUILayout* layer)
{
	m_LayersToAdjust.push_back(layer);
}
