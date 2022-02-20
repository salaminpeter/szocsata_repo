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
		m_Children[i]->SetPosition(m_LayoutBoxes[i].m_BottomLeftX, m_LayoutBoxes[i].m_BottomLeftY, false);

		float WidthDiff = m_Children[i]->GetWidth() - m_LayoutBoxes[i].m_Width;
		float HeightDiff = m_Children[i]->GetHeight() - m_LayoutBoxes[i].m_Height;

		//resize element if needed
		if ((m_LayoutBoxes[i].m_IncSizeAllowed && (WidthDiff < -1.f || HeightDiff < -1.f)) || (WidthDiff > 1.f || HeightDiff > 1.f))
			m_Children[i]->Resize(m_LayoutBoxes[i].m_Width, m_LayoutBoxes[i].m_Height);
	}

	m_LayoutDone = true;

	//adjust "child" layouts
	for (size_t i = 0; i < m_LayersToAdjust.size(); ++i)
		m_LayersToAdjust[i]->AdjustToLayer();

}

bool CUILayout::GetBoxProperties(size_t idx, int& x, int& y, int& w, int& h, int& minGap, int& maxGap, int& gap, int& maxW, int& maxH, bool& inc, float& whRatio)
{
	if (idx >= m_LayoutBoxes.size())
		return false;

	x = m_LayoutBoxes[idx].m_BottomLeftX;
	y = m_LayoutBoxes[idx].m_BottomLeftY;
	w = m_LayoutBoxes[idx].m_Width;
	h = m_LayoutBoxes[idx].m_Height;
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
		m_AdjustToLayer->GetBoxProperties(i, x, y, w, h, minGap, maxGap, gap, maxW, maxH, inc, whRatio);

		m_LayoutBoxes[i].m_BottomLeftX = x;
		m_LayoutBoxes[i].m_BottomLeftY = y;
		m_LayoutBoxes[i].m_Width = w;
		m_LayoutBoxes[i].m_Height = h;
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

void CUILayout::SetBoxWHRatio(size_t idx, float whRatio)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_WHRatio = whRatio;
}


void CUILayout::SetBoxSizeProps(size_t idx, int maxWidth, int maxHeight, bool incSize)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_MaxWidth = maxWidth;
	m_LayoutBoxes[idx].m_MaxHeight = maxHeight;
	m_LayoutBoxes[idx].m_Width = maxWidth;
	m_LayoutBoxes[idx].m_Height = maxHeight;
	m_LayoutBoxes[idx].m_WHRatio = float(maxWidth) / float(maxHeight);
	m_LayoutBoxes[idx].m_IncSizeAllowed = incSize;
}

void CUILayout::SetBoxGapProps(size_t idx, int minGap, int maxGap)
{
	if (idx >= m_LayoutBoxes.size())
		return;

	m_LayoutBoxes[idx].m_MinGap = minGap;
	m_LayoutBoxes[idx].m_MaxGap = maxGap;
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
			MaxHeight = m_LayoutBoxes[i].m_Height;

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

void CUILayout::ResizeElement(float widthPercent, float heightPercent)
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		m_LayoutBoxes[i].m_Width = m_Children[i]->GetWidth();
		m_LayoutBoxes[i].m_MaxWidth = m_Children[i]->GetWidth();
		m_LayoutBoxes[i].m_Height = m_Children[i]->GetHeight();
		m_LayoutBoxes[i].m_MaxHeight = m_Children[i]->GetHeight();
		m_LayoutBoxes[i].m_WHRatio = float(m_LayoutBoxes[i].m_Width) / float(m_LayoutBoxes[i].m_Height);
	}

	AlignChildren();
}
