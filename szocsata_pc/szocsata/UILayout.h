#pragma once

#include "UIElement.h"
#include "Model.h"

class CUILayout : public CUIElement
{
public:

	CUILayout(float x, float y, float w, float h, int vx, int vy, CUIElement* parent, const wchar_t* id) :
		CUIElement(parent, id, new CModel(false, 0, nullptr, nullptr), x, y, w, h, vx, vy, 0.f, 0.f)
	{}

	void SetBoxAlignProps(size_t idx, int alignmentH, int alignmentV, bool incSize);
	void SetBoxGapProps(size_t idx, int minGap, int maxGap);
	void SetBoxSizeProps(size_t idx, int maxWidth, int maxHeight);

protected:

	void LayoutChildren();
	float GetGapSum(bool min);
	float GetGapSum();
	float GetHeightSum();
	float GetWidthSum();
	float GetHeightGapSum();
	float GetMaxWidth();
	float GetMaxHeight();

protected:

	struct TLayoutBox
	{
		int m_BottomLeftX;
		int m_BottomLeftY;
		int m_Width;
		int m_Height;
		int m_AlignmentH;
		int m_AlignmentV;
		int m_MinGap;
		int m_MaxGap;
		int m_Gap;
		int m_MaxWidth;
		int m_MaxHeight;
		bool m_IncSizeAllowed;
		float m_WHRatio;

		TLayoutBox(float x, float y, int minGap, int maxGap, int maxWidth, int maxHeight, float whRatio, bool incSize, int alignH, int alignV) :
			m_BottomLeftX(x), 
			m_BottomLeftY(y), 
			m_Width(maxWidth), 
			m_Height(maxWidth * whRatio), 
			m_MinGap(minGap),
			m_MaxGap(maxGap),
			m_MaxWidth(maxWidth),
			m_MaxHeight(maxHeight),
			m_WHRatio(whRatio),
			m_Gap(0),
			m_IncSizeAllowed(incSize),
			m_AlignmentH(alignH),
			m_AlignmentV(alignV)
			{}
	};

	std::vector<TLayoutBox> m_LayoutBoxes;	
};