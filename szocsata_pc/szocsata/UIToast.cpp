#include "stdafx.h"
#include "UIToast.h"
#include "TimerEventManager.h"
#include "UIManager.h"

int CUIToast::m_TimeSinceShow = 0;

void CUIToast::StartTimer()
{
	m_TimerEventManager->AddTimerEvent(m_UIManager, &CUIManager::CloseToast, nullptr, "ui_toast_id");
	m_TimerEventManager->StartTimer("ui_toast_id");
}

