#include "stdafx.h"
#include "UIPlayerLetterPanel.h"


bool CUIPlayerLetterPanel::HandleEventAtPos(int x, int y, EEventType event, CUIElement* root, bool checkChildren, bool selfCheck)
{
	return m_Children[0]->HandleEventAtPos(x, y, event, root, checkChildren);
}

bool CUIPlayerLetterPanel::GetLetterSize(float& size)
{
	if (m_Children.size() == 0 || m_Children[0]->GetChildCount() == 0)
		return false;

	size = m_Children[0]->GetChild(size_t(0))->GetWidth();

	return true;
}
