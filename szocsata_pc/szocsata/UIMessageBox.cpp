#include "stdafx.h"
#include "UIMessageBox.h"
#include "UIButton.h"
#include "UIText.h"
#include "SquareModelData.h"
#include "Model.h"


CUIMessageBox::CUIMessageBox(const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id) :
	CUIElement(nullptr, id, new CModel(false, 2, positionData.get(), colorData.get(), textureID, "textured"), x, y, w, h, vx, vy, 0.f, 0.f)
{
	float BtnSize = h / 4;
	float BtnX = (w - BtnSize) / 2;
	float BtnY = (h - BtnSize) - (h / 10);

	m_OkButton = std::make_unique<CUIButton>(positionData, colorData, BtnX, BtnY, BtnSize, BtnSize, vx, vy, "okbutton.bmp", id);
	m_OkButton->PositionElement();
	PositionElement();
}
