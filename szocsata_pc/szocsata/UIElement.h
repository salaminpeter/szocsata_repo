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
	
	enum EEventType { TouchEvent, ReleaseEvent, PositionChangeEvent, DoubleClickEvent };
	enum EAlignmentType { Center, Left, Right, Top, Bottom, None };

	CUIElement(CUIElement*  parent, const wchar_t* id, CModel *model, int x, int y, int w, int h, int vx, int vy, float tx, float ty);
	virtual ~CUIElement();
		
	template <typename ClassType, typename... ArgTypes>
	void SetEvent(EEventType event, ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, ArgTypes&&... args)
	{
		//touch event
		if (event == EEventType::TouchEvent)
			m_EventTouch = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
		//release event
		else if (event == EEventType::ReleaseEvent)
			m_EventRelease = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
		//double click event
		else if (event == EEventType::DoubleClickEvent)
			m_EventDoubleClick = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
	}

	bool HandleEvent(EEventType event);
	bool PositionInElement(int x, int y);
	void PositionElement();
	void AddChild(CUIElement* child);
	void RemoveLastChild();
	void SetPosAndSize(float x, float y, float w, float h, bool midPointOrigo = true, bool scaleChildren = false);
	void SetPosition(float x, float y, bool midPointOrigo = true);
	void SetSizeWithChildren(float width, float height, float xScale = 0.f, float yScale = 0.f, int depth = 0);

	glm::vec2 GetPosition(bool midPointOrigo = true);
	glm::vec2 GetRelativePosition(glm::vec2 pos);
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
	void Scale(float scale);

	virtual void Render(CRenderer* renderer);
	virtual bool HandleEventAtPos(int x, int y, EEventType event, CUIElement* root = nullptr, bool checkChildren = true, bool selfCheck = true);
	virtual void AlignChildren(); 

protected:
	virtual void ResizeElement(float widthPercent, float heightPercent) {}

public:
	std::vector<CUIElement*> m_Children;
	bool m_IsLayout = false;

private:
	void RenderInner(CRenderer* renderer, int& elemIdx, int& colorBufferID, int& textureOffset, int elemCount = 0);

protected:
	
	const float m_MinResizeDif = 4.f;

	CUIElement* m_Parent = nullptr;
	CModel* m_Model = nullptr;
	CEventBase* m_EventTouch = nullptr;
	CEventBase* m_EventRelease = nullptr;
	CEventBase* m_EventDoubleClick = nullptr;

	int m_ViewXPosition;
	int m_ViewYPosition;
	float m_XPosition;
	float m_YPosition;
	float m_Width;
	float m_Height;
	bool m_Visible = true;
	bool m_Enabled = true;
	bool m_KeepAspect = true;
	
	bool m_CheckChildEvents = true;

	glm::vec4 m_TextureModColor = glm::vec4(1, 1, 1, 1);
	glm::vec2 m_TexturePosition;
	std::wstring m_ID;
};