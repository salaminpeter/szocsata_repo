#include "stdafx.h"
#include "UIToast.h"
#include "TimerEventManager.h"
#include "UIManager.h"

int CUIToast::m_TimeSinceShow = 0;

CUIToast::CUIToast(unsigned timeout, CTimerEventManager* timerEventManager, CUIManager* uiManager, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type, CGameManager* gameManager) :
	CUIMessageBox(positionData, colorData, gridColorData, x, y, w, h, vx, vy, NoButton, gameManager),
	m_TimerEventManager(timerEventManager),
	m_UIManager(uiManager)
{
	m_TimerEventManager->AddTimerEvent(m_UIManager, &CUIManager::CloseToast, nullptr, "ui_toast_id");
}


void CUIToast::StartTimer()
{
	m_TimerEventManager->StartTimer("ui_toast_id");
}

