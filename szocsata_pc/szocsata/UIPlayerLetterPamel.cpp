#include "stdafx.h"
#include "UIPlayerLetterPanel.h"


bool CUIPlayerLetterPanel::HandleEventAtPos(int x, int y, bool touchEvent, CUIElement* root, bool checkChildren)
{
	return m_Children[0]->HandleEventAtPos(x, y, touchEvent, root, checkChildren);
}
