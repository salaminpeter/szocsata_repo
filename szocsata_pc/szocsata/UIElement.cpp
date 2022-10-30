#include "stdafx.h"
#include "UIElement.h"
#include "Model.h"
#include "Renderer.h"


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
	delete m_EventDoubleClick;

	DeleteRecursive();
}

bool CUIElement::HandleEvent(EEventType event)
{
	if (!m_Visible || !m_Enabled)
		return false;

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
	else if (event == EEventType::DoubleClickEvent)
	{		
		if (m_EventDoubleClick)
		{
			m_EventDoubleClick->HandleEvent();
			return true;
		}
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
		AbsPos.x += Parent->GetPosition().x;
		AbsPos.y += Parent->GetPosition().y;
		Parent = Parent->GetParent();
	}

	return AbsPos;
}

void CUIElement::Resize(float newWidth, float newHeight, int depth)
{
	if (m_Width == 0 || m_Height == 0)
		return;

	if (m_KeepAspect)
	{
		float ResizedHeight = (float(m_Height) / float(m_Width)) * newWidth;
		float ResizedWidth = newWidth;

		if (ResizedHeight > newHeight)
		{
			ResizedHeight = newHeight;
			ResizedWidth = (float(m_Width) / float(m_Height)) * newHeight;
		}
				
		newWidth = ResizedWidth;
		newHeight = ResizedHeight;
	}

	float WidthPercent = newWidth / m_Width;
	float HeightPercent = newHeight / m_Height;

	float NewXPos = m_Parent && depth != 0 ? m_XPosition * WidthPercent : m_XPosition;
	float NewYPos = m_Parent && depth != 0 ? m_YPosition * HeightPercent : m_YPosition;

	SetPosAndSize(NewXPos, NewYPos, newWidth, newHeight);

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->Resize(m_Children[i]->GetWidth() * WidthPercent, m_Children[i]->GetHeight() * HeightPercent, depth + 1);

	ResizeElement(WidthPercent, HeightPercent);
}

int CUIElement::GetVisibleElemCount()
{
	if (!m_Visible)
		return 0;
	
	int Count = 0;

	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		Count += m_Children[i]->GetVisibleElemCount();
	}

	return Count + (m_Model && m_Model->HasTexture() ? 1 : 0);
}


void CUIElement::RenderInner(CRenderer* renderer, int& elemIdx, int& colorBufferID, int& textureOffset, int elemCount)
{
	if (!m_Visible)
		return;
	
	if (m_Model && m_Model->HasTexture())
	{
		bool BindTexture = colorBufferID != m_Model->GetColorBufferID();
		bool SetTextureVertexAttrib = BindTexture || textureOffset != m_Model->GetTextureOffset();

		colorBufferID = m_Model->GetColorBufferID();
		textureOffset = m_Model->GetTextureOffset();

		renderer->SetModifyColor(m_TextureModColor.r, m_TextureModColor.g, m_TextureModColor.b, m_TextureModColor.a);
		renderer->SetTexturePos(m_TexturePosition);
		renderer->DrawModel(m_Model, "view_ortho", "textured", false, elemIdx == 0, BindTexture, elemIdx == elemCount - 1, SetTextureVertexAttrib);
		elemIdx++;
	}

	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		m_Children[i]->RenderInner(renderer, elemIdx, colorBufferID, textureOffset, elemCount);
	}
}

void CUIElement::Render(CRenderer* renderer) 
{
	int VisibleElements = GetVisibleElemCount();
	int RenderedElements = 0;
	int ColorBufferID = -1;
	int TextureOffset = -1;

	RenderInner(renderer, RenderedElements, ColorBufferID, TextureOffset, VisibleElements);
};

void CUIElement::AlignChildren()
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		m_Children[i]->AlignChildren();
	}
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

//get positon of pos relative to the elem (pos has to be absolute position!)
glm::vec2 CUIElement::GetRelativePosition(glm::vec2 pos)
{
	return pos - GetAbsolutePosition();
}

glm::vec2 CUIElement::GetPosition(bool midPointOrigo)
{
	if (midPointOrigo)
		return glm::vec2(m_XPosition, m_YPosition);

	bool RootElem = !m_Parent->GetParent();
	float ParentWidth = m_Parent ? m_Parent->GetWidth() : 0;
	float ParentHeight = m_Parent ? m_Parent->GetHeight() : 0;

	if (!RootElem)
		return glm::vec2(ParentWidth / 2 + m_XPosition - m_Width / 2, ParentHeight / 2 + m_YPosition - m_Height / 2);
	else
		return glm::vec2(m_XPosition - m_Width / 2, m_YPosition - m_Height / 2);
}

void CUIElement::SetSizeWithChildren(float width, float height, float xScale, float yScale, int depth)
{
	float WidthScale = depth == 0 ? width / m_Width : xScale;
	float HeigthScale = depth == 0 ? height / m_Height : yScale;

	if (depth == 0)
	{
		m_Width = width;
		m_Height = height;
		PositionElement();
	}
	else
	{
		float RelativeXPos = GetPosition().x * xScale;
		float RelativeYPos = GetPosition().y * yScale;
		SetWidth(m_Width * xScale);
		SetHeight(m_Height * yScale);
		SetPosition(RelativeXPos, RelativeYPos);
	}

	for (auto ChildElement : m_Children)
		ChildElement->SetSizeWithChildren(0.f, 0.f, WidthScale, HeigthScale, depth + 1);
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

void CUIElement::SetPosAndSize(float x, float y, float w, float h, bool midPointOrigo, bool scaleChildren)
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
