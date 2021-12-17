#include "stdafx.h"
#include "UIElement.h"
#include "Model.h"


CUIElement::CUIElement(CUIElement* parent, const wchar_t* id, CModel* model, int x, int y, int w, int h, int vx, int vy, float tx, float ty) :
	m_Parent(parent),
	m_Model(model),
	m_XPosition(x),
	m_YPosition(y),
	m_Width(w),
	m_Height(h),
	m_ViewXPosition(vx),
	m_ViewYPosition(vy),
	m_TexturePosition(tx, ty),
	m_ID(id)
{
	if (parent)
		parent->AddChild(this);
}

CUIElement::~CUIElement()
{
	delete m_Model;
	delete m_EventTouch;
	delete m_EventRelease;
	DeleteRecursive();
}

bool CUIElement::HandleEvent(EEventType event)
{
	if (event == EEventType::TouchEvent && m_EventTouch)
	{ 
		m_EventTouch->HandleEvent();
		return true;
	}
	else if (event == EEventType::ReleaseEvent && m_EventRelease)
	{
		m_EventRelease->HandleEvent();
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

bool CUIElement::HandleEventAtPos(int x, int y, EEventType event, CUIElement* root, bool checkChildren, bool selfCheck)
{ 
	if (!root)
		root = this;

	//ha nem lathato az elem, vagy nincs enengedelyezve
	if (!m_Visible || !m_Enabled)
		return false;

	//gyerek controllokra vegigellenorizzuk tudja e kezelni valamelyik az esemenyt (belulrol kifele)
	if (checkChildren && root->m_CheckChildEvents)
	{
		for (size_t i = 0; i < root->m_Children.size(); ++i)
		{
			if (HandleEventAtPos(x, y, event, root->m_Children[i]))
				return true;
		}
	}

	//a control custom lekezelte az esemenyt a sajat HandleEventAtPos fugvenyevel
	if (selfCheck && root->HandleEventAtPos(x, y, event, root, false, false))
		return true;
	
	//ha nincs custom fuggveny megnezzuk hogy a controllban van e a pozicio, es tusjuk e kezelni az esemenyt
	else if (root->PositionInElement(x, y))
	{
		if (root->HandleEvent(event))
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

void CUIElement::SetPosition(float x, float y, bool midPointOrigo)
{
	if (midPointOrigo)
	{
		m_XPosition = x;
		m_YPosition = y;
	}
	else
	{
		bool RootElem = !m_Parent->GetParent();
		float ParentWidth = m_Parent ? m_Parent->GetWidth() : 0;
		float ParentHeight = m_Parent ? m_Parent->GetHeight() : 0;

		if (!RootElem)
		{
			m_XPosition = x - ParentWidth / 2 + m_Width / 2;
			m_YPosition = y - ParentHeight / 2 + m_Height / 2;
		}
		else
		{
			m_XPosition = x + m_Width / 2;
			m_YPosition = y + m_Height / 2;
		}
	}

	PositionElement();
}

void CUIElement::SetPosAndSize(float x, float y, float w, float h, bool midPointOrigo)
{
	m_Width = w;
	m_Height = h;

	SetPosition(x, y, midPointOrigo);
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
	{
		if (m_Children[i]->GetID() == childIdx)
			return m_Children[i];

		CUIElement* FoundChild = m_Children[i]->GetChild(childIdx);

		if (FoundChild)
			return FoundChild;
	}

	return nullptr;
}

CUIElement* CUIElement::GetChild(size_t childIdx) 
{ 
	if (childIdx >= m_Children.size())
		return nullptr;

	return m_Children[childIdx];
}
