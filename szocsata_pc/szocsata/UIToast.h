#pragma once

#include "UIMessageBox.h"

class CTimerEventManager;
class CUIManager;
class CGameManager;

class CUIToast : public CUIMessageBox
{
public:

	CUIToast(unsigned timeout, CTimerEventManager* timerEventManager, CUIManager* uiManager, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type, CGameManager* gameManager);

	void StartTimer();

	static int m_TimeSinceShow;

private:
	
	CTimerEventManager* m_TimerEventManager;
	CUIManager* m_UIManager;
};
