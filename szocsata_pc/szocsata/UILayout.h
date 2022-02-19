#pragma once

#include "UIElement.h"
#include "Model.h"

class CUILayout : public CUIElement
{
public:

	CUILayout(float x, float y, float w, float h, int vx, int vy, CUIElement* parent, const wchar_t* id, int elemCount, float whRatio = 1.f, int minGap = 0.f, int maxGap = 0.f, int maxW = 0.f, int maxH = 0.f) :
		CUIElement(parent, id, new CModel(false, 0, nullptr, nullptr), x, y, w, h, vx, vy, 0.f, 0.f)
	{
		m_KeepAspect = false;
	}

	void SetBoxGapProps(size_t idx, int minGap, int maxGap);
	void SetBoxSizeProps(size_t idx, int maxWidth, int maxHeight, bool incSize);
	void SetBoxWHRatio(size_t idx, float whRatio);

	bool GetBoxProperties(size_t idx, int& x, int& y, int& w, int& h, int& minGap, int& maxGap, int& gap, int& maxW, int& maxH, bool& inc, float& whRatio);

	void SetAdjustToLayer(CUILayout* layer);
	void AddLayerToAdjust(CUILayout* layer);
	void AdjustToLayer();

	void ResizeElement(float widthPercent, float heightPercent) override;

	glm::ivec2 GetLayoutBoxPosition(unsigned idx) const { return glm::ivec2(m_LayoutBoxes[idx].m_BottomLeftX, m_LayoutBoxes[idx].m_BottomLeftY);}

protected:

	void LayoutChildren();
	float GetGapSum(bool min);
	float GetGapSum();
	float GetHeightSum();
	float GetWidthSum();
	float GetMaxWidth();
	float GetMaxHeight();

	bool IsLayoutDone() { return m_LayoutDone; }
	bool IsAdjustedToLayout() { return m_AdjustToLayer != nullptr; }

	virtual void PositionLayoutBoxes() {}

protected:

	struct TLayoutBox
	{
		int m_BottomLeftX;
		int m_BottomLeftY;
		int m_Width;
		int m_Height;
		int m_MinGap;
		int m_MaxGap;
		int m_Gap;
		int m_MaxWidth;
		int m_MaxHeight;
		bool m_IncSizeAllowed;
		float m_WHRatio;

		TLayoutBox(float x, float y, int minGap, int maxGap, int maxWidth, int maxHeight, float whRatio, bool incSize) :
			m_BottomLeftX(x), 
			m_BottomLeftY(y), 
			m_Width(maxWidth), 
			m_Height(maxWidth / whRatio), 
			m_MinGap(minGap),
			m_MaxGap(maxGap),
			m_MaxWidth(maxWidth),
			m_MaxHeight(maxHeight),
			m_WHRatio(whRatio),
			m_Gap(0),
			m_IncSizeAllowed(incSize)
		{}
	};

	std::vector<TLayoutBox> m_LayoutBoxes;	

	CUILayout* m_AdjustToLayer = nullptr;
	std::vector<CUILayout*> m_LayersToAdjust;

	bool m_LayoutDone = false;
};