#include "stdafx.h"
#include "UIElement.h"
#include "Model.h"


CUIElement::CUIElement(CUIElement* parent, const wchar_t* id, CModel* model, int x, int y, int w, int h, int vx, int vy, float tx, float ty, bool ignoreReleaseEvent) :
	m_Parent(parent),
	m_Model(model),
	m_XPosition(x),
	m_YPosition(y),
	m_Width(w),
	m_Height(h),
	m_ViewXPosition(vx),
	m_ViewYPosition(vy),
	m_TexturePosition(tx, ty),
	m_ID(id),
	m_IgnoreReleaseEvent(ignoreReleaseEvent)
{
	if (parent)
		parent->AddChild(this);
}

void CUIElement::HandleEvent()
{
	if (m_Event)
		m_Event->HandleEvent();
}

bool CUIElement::PositionInElement(int x, int y)
{
	x -= m_ViewXPosition;
	y -= m_ViewYPosition;

	return x >= m_XPosition - m_Width / 2 && x <= m_XPosition + m_Width / 2 && y >= m_YPosition - m_Height / 2 && y <= m_YPosition + m_Height / 2; //TODO check child elements!
}

void CUIElement::PositionElement()
{
	if (m_Model)
	{
		m_Model->ResetMatrix();
		m_Model->Translate(glm::vec3(m_XPosition, m_YPosition, 0.f));
		m_Model->Scale(glm::vec3(m_Width, m_Height, 1.f));
	}

	//TODO childelements!
}

void CUIElement::DeleteRecursive()
{
	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->DeleteRecursive();

	delete this;
}


void CUIElement::SetPosition(float x, float y)
{
	m_XPosition = x;
	m_YPosition = y;

	PositionElement();
}

void CUIElement::SetPosAndSize(float x, float y, float w, float h)
{
	m_XPosition = x;
	m_YPosition = y;
	m_Width = w;
	m_Height = h;

	PositionElement();
}


void CUIElement::AddChild(CUIElement* child)
{
	m_Children.push_back(child);
}

void CUIElement::RemoveLastChild()
{
	m_Children.pop_back();
}

void CUIElement::Scale(float scale) 
{ 
	m_Model->SetScale(glm::vec3(scale)); 
}

CModel* CUIElement::GetModel()
{
	return m_Model;
}

CModel* CUIElement::GetModel(size_t idx)
{
	if (idx >= m_Children.size())
		return nullptr;

	return m_Children[idx]->GetModel();
}

CUIElement* CUIElement::GetChild(const wchar_t* childIdx)
{
	for (size_t i = 0; i < m_Children.size(); ++i)
		if (m_Children[i]->GetID() == childIdx)
			return m_Children[i];

	return nullptr;
}

CUIElement* CUIElement::GetChild(size_t childIdx) 
{ 
	if (childIdx >= m_Children.size())
		return nullptr;

	return m_Children[childIdx];
}
