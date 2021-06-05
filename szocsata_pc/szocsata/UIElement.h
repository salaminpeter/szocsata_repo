#pragma once

#include "glm\gtc\matrix_transform.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Event.h"

class CModel;
class CRenderer;

class CUIElement
{
friend CUIElement;

public:

	CUIElement(CUIElement*  parent, const wchar_t* id, CModel *model, int x, int y, int w, int h, int vx, int vy, float tx, float ty);
		
	template <typename ClassType, typename... ArgTypes>
	void SetEvent(ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, ArgTypes&&... args)
	{
		m_Event = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
	}

	void HandleEvent();
	bool PositionInElement(int x, int y);
	void PositionElement();
	void AddChild(CUIElement* child);
	void RemoveLastChild();
	void SetPosAndSize(float x, float y, float w, float h);
	CModel* GetModel();
	CModel* GetModel(size_t idx);

	glm::vec2 GetTexturePos() {
		return m_TexturePosition;
	}
	glm::vec2 GetTexturePos(size_t idx)
	{
		return m_Children[idx]->GetTexturePos();
	}
	void SetTexturePosition(glm::vec2 texPos) {m_TexturePosition = texPos;}
	void SetVisible(bool visible) {m_Visible = visible;}
	void SetVisible(bool visible, size_t idx) { m_Children[idx]->SetVisible(visible); }
	bool IsVisible() {return m_Visible;}
	bool IsVisible(size_t idx) { return m_Children[idx]->m_Visible; }
	std::wstring GetID() {return m_ID;}
	CUIElement* GetChild(size_t childIdx);

	virtual void Render(CRenderer* renderer) {};


protected:

	std::shared_ptr<CUIElement> m_Parent;
	CModel* m_Model = nullptr;
	CEventBase* m_Event = nullptr;

	std::vector<CUIElement*> m_Children;

	int m_ViewXPosition;
	int m_ViewYPosition;
	int m_XPosition;
	int m_YPosition;
	int m_Width;
	int m_Height;
	bool m_Visible = true;
	glm::vec2 m_TexturePosition;
	std::wstring m_ID;

	public:
	std::string m_Type; //TODO szepen!
};