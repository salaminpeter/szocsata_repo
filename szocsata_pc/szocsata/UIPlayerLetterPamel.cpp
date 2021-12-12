#include "stdafx.h"
#include "UIPlayerLetterPanel.h"


bool CUIPlayerLetterPanel::HandleEventAtPos(int x, int y, EEventType event, CUIElement* root, bool checkChildren, bool selfCheck)
{
	return m_Children[0]->HandleEventAtPos(x, y, event, root, checkChildren);
}
