#include "stdafx.h"
#include "UIPanel.h"
#include "UIText.h"
#include "UIButton.h"
#include "Model.h"
#include "Renderer.h"


CUIPanel::CUIPanel(CUIElement* parent, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty) :
	CUIElement(parent, id, new CModel(false, 2, (CModelPositionData*)(positionData.get()), (CModelColorData*)(colorData.get()), textureID, "textured"), x, y, w, h, vx, vy, 0.f, 0.f)
{
	SetPosAndSize(x, y, w, h);
}

void CUIPanel::AddText(const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* textureID, const wchar_t* id)
{
	//TODO szepen!!!
	new CUIText(this, positionData, colorData, text, x + m_XPosition, y + m_YPosition, w, h, m_ViewXPosition, m_ViewYPosition, id);
}

void CUIPanel::AddButton(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* textureID, const wchar_t* id)
{
	//TODO szepen!!!
	new CUIButton(this, positionData, colorData, x + m_XPosition, y + m_YPosition, w, h, m_ViewXPosition, m_ViewYPosition, textureID, id);
}

void CUIPanel::HandleTouchEvent(int x, int y)
{
	for (size_t i = 1; i < m_Children.size(); ++i)
	{
		CUIButton* Button = static_cast<CUIButton*>(m_Children[i]);

		if (Button->PositionInElement(x, y))
		{
			Button->HandleEvent();
			return;
		}
	}
}

void CUIPanel::Render(CRenderer* renderer)
{
	if (!m_Visible)
		return;

	//draw panel
	renderer->SetTexturePos(glm::vec2(0.f, 0.f));
	renderer->DrawModel(m_Model, "view_ortho", "textured", false);

	//draw child ui elements
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		m_Children[i]->Render(renderer);
	}
}


