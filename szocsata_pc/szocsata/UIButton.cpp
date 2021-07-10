#include "stdafx.h"
#include "UIButton.h"
#include "SquareModelData.h"
#include "Model.h"
#include "Renderer.h"


CUIButton::CUIButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, float vx, float vy, const char* textureID, const wchar_t* id, bool ignoreReleaseEvent) :
	CUIElement(parent, id, new CModel(false, 2, positionData.get(), colorData.get(), textureID, "textured"), x, y, w, h, vx, vy, 0.f, 0.f, ignoreReleaseEvent)
{
	PositionElement();
}

bool CUIButton::HandleEventAtPos(int x, int y, bool touchEvent)
{
	if ((touchEvent || !m_IgnoreReleaseEvent) && m_Visible && m_Enabled && PositionInElement(x, y))
	{
		HandleEvent();
		return true;
	}

	return false;
}


void CUIButton::Render(CRenderer* renderer)
{
	if (!m_Visible)
		return;

	renderer->SetTexturePos(m_TexturePosition);
	renderer->DrawModel(m_Model, "view_ortho", "textured", false);
}
