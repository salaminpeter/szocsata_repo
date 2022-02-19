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
	virtual ~CUIElement();
		
	template <typename ClassType, typename... ArgTypes>
	void SetEvent(bool touch, ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, ArgTypes&&... args)
	{
		if (touch)
			m_EventTouch = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
		else
			m_EventRelease = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
	}

	enum EEventType { TouchEvent, ReleaseEvent, PositionChangeEvent };
	enum EAlignmentType { Center, Left, Right, Top, Bottom, None};

	bool HandleEvent(EEventType event);
	bool PositionInElement(int x, int y);
	void PositionElement();
	void AddChild(CUIElement* child);
	void RemoveLastChild();
	void SetPosAndSize(float x, float y, float w, float h, bool midPointOrigo = true);
	void SetPosition(float x, float y, bool midPointOrigo = true);
	glm::vec2 GetPosition(bool midPointOrigo = true);
	void DeleteRecursive();

	glm::vec2 GetAbsolutePosition();

	CModel* GetModel();
	CModel* GetModel(size_t idx);

	float GetHeight() { return m_Height; }
	float GetWidth() { return m_Width; }
	void SetWidth(float w) { m_Width = w; }
	void SetHeight(float h) { m_Height = h; }

	glm::vec2 GetTexturePos() {	return m_TexturePosition;}
	glm::vec2 GetTexturePos(size_t idx) { return m_Children[idx]->GetTexturePos();}

	void Scale(float scale);
	void SetTexturePosition(glm::vec2 texPos) {m_TexturePosition = texPos;}
	void SetVisible(bool visible) {m_Visible = visible;}
	void SetVisible(bool visible, size_t idx) { m_Children[idx]->SetVisible(visible); }
	bool IsVisible() {return m_Visible;}
	bool IsVisible(size_t idx) { return m_Children[idx]->m_Visible; }
	std::wstring GetID() const {return m_ID;}
	int GetChildCount() {return m_Children.size();}
	CUIElement* GetChild(size_t childIdx);
	CUIElement* GetChild(const wchar_t* childIdx);
	CUIElement* GetParent() {return m_Parent;}
	void Enable(bool enable) { m_Enabled = enable; }
	bool IsEnabled() { return m_Enabled; }
	void SetCheckChildEvent(bool b) { m_CheckChildEvents = b; }
	void SetModifyColor(glm::vec4 modColor) {m_TextureModColor = modColor;}
	void SetKeepAspect(bool keep) {m_KeepAspect = keep;}
	int GetVisibleElemCount();
	void Resize(float newWidth, float newHeight, int depth = 0);

	virtual void Render(CRenderer* renderer);
	virtual bool HandleEventAtPos(int x, int y, EEventType event, CUIElement* root = nullptr, bool checkChildren = true, bool selfCheck = true);

	virtual void AlignChildren() 
	{
		for (size_t i = 0; i < m_Children.size(); ++i)
		{
			m_Children[i]->AlignChildren();
		}
	}

protected:

	virtual void ResizeElement(float widthPercent, float heightPercent) {}

public:
	std::vector<CUIElement*> m_Children;

private:
	void RenderInner(CRenderer* renderer, int& elemIdx, int& colorBufferID, int& textureOffset, int elemCount = 0);

protected:
	
	const float m_MinResizeDif = 4.f;

	CUIElement* m_Parent = nullptr;
	CModel* m_Model = nullptr;
	CEventBase* m_EventTouch = nullptr;
	CEventBase* m_EventRelease = nullptr;

	int m_ViewXPosition;
	int m_ViewYPosition;
	int m_XPosition;
	int m_YPosition;
	int m_Width;
	int m_Height;
	bool m_Visible = true;
	bool m_Enabled = true;
	bool m_KeepAspect = true;
	
	bool m_CheckChildEvents = true;

	glm::vec4 m_TextureModColor = glm::vec4(1, 1, 1, 1);
	glm::vec2 m_TexturePosition;
	std::wstring m_ID;
};