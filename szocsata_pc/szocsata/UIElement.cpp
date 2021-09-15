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

CUIElement::~CUIElement()
{
	delete m_Model;
	delete m_Event;
	DeleteRecursive();
}

bool CUIElement::HandleEvent()
{
	if (m_Event)
	{ 
		m_Event->HandleEvent();
		return true;
	}

	return false;
}

bool CUIElement::PositionInElement(int x, int y)
{
	x -= m_ViewXPosition;
	y -= m_ViewYPosition;

	glm::vec2 AbsPos = GetAbsolutePosition();

	return x >= AbsPos.x - m_Width / 2 && x <= AbsPos.x + m_Width / 2 && y >= AbsPos.y - m_Height / 2 && y <= AbsPos.y + m_Height / 2; //TODO check child elements!
}

glm::vec2 CUIElement::GetAbsolutePosition()
{
	glm::vec2 AbsPos(m_XPosition, m_YPosition);
	CUIElement* Parent = m_Parent;

	while (Parent)
	{
		AbsPos.x += Parent->GetXPosition();
		AbsPos.y += Parent->GetYPosition();
		Parent = Parent->GetParent();
	}

	return AbsPos;
}

bool CUIElement::HandleEventAtPos(int x, int y, bool touchEvent, CUIElement* root, bool checkChildren)
{ 
	if (!root)
		root = this;
	else if (root == this)
		return false;

	//ha nem lathato az elem, vagy nincsenengedelyezve vagy nem kezel realeseeventet
	if ((!touchEvent && m_IgnoreReleaseEvent) || !m_Visible || !m_Enabled)
		return false;

	//gyerek controllokra vegigellenorizzuk tudja e kezelni valamelyik az esemenyt (belulrol kifele)
	if (checkChildren)
	{
		for (size_t i = 0; i < root->m_Children.size(); ++i)
		{
			if (HandleEventAtPos(x, y, touchEvent, root->m_Children[i]))
				return true;
		}
	}

	//a control custom lekezelte az esemenyt a sajat HandleEventAtPos fugvenyevel
	if (root->HandleEventAtPos(x, y, touchEvent, root, false))
		return true;
	
	//ha nincs custom fuggveny megnezzuk hogy a controllban van e a pozicio, es tusjuk e kezelni az esemenyt
	else if ((touchEvent || !touchEvent && !m_IgnoreReleaseEvent) && root->PositionInElement(x, y))
	{
		if (root->HandleEvent())
			return true;
	}

	return false;
}

void CUIElement::PositionElement()
{
	if (m_Model)
	{
		glm::vec2 AbsPos = GetAbsolutePosition();

		m_Model->ResetMatrix();
		m_Model->Translate(glm::vec3(AbsPos, 0.f));
		m_Model->Scale(glm::vec3(m_Width, m_Height, 1.f));
	}

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->PositionElement();
}

void CUIElement::DeleteRecursive()
{
	for (size_t i = 0; i < m_Children.size(); ++i)
		delete m_Children[i];

	m_Children.clear();
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
