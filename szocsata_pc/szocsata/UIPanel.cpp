#include "stdafx.h"
#include "UIPanel.h"
#include "UIText.h"
#include "UIButton.h"
#include "Model.h"
#include "ModelPositionData.h"
#include "SquareModelData.h"
#include "Renderer.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>


CUIPanel::CUIPanel(CUIElement* parent, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty) :
	CUIElement(parent, id, new CModel(false, 2, std::static_pointer_cast<CModelPositionData>(positionData), std::static_pointer_cast<CModelColorData>(colorData), textureID, "textured"), x, y, w, h, vx, vy, 0.f, 0.f),
	m_PositionData(positionData),
	m_ColorData(colorData),
	m_GridColorData(gridColorData)
{
	SetPosAndSize(x, y, w, h);
}

void CUIPanel::AddText(const wchar_t* text, float x, float y, float fontHeight, const char* textureID, const wchar_t* id, float r, float g, float b)
{
	new CUIText(this, m_PositionData, m_GridColorData, text, fontHeight, x, y, m_ViewXPosition, m_ViewYPosition, r, g, b, id);
}

void CUIPanel::AddButton(float x, float y, float w, float h, const char* textureID, const wchar_t* id)
{
	new CUIButton(this, m_PositionData, m_ColorData, x, y, w, h, m_ViewXPosition, m_ViewYPosition, textureID, id);
}

