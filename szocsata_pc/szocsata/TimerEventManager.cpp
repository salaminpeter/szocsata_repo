#include "stdafx.h"
#include "TimerEventManager.h"
#include "Timer.h"
#include <algorithm>


void CTimerEventManager::Loop()
{
	if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
		return;
	
	auto it = m_TimerEvents.begin();
	
	while (it != m_TimerEvents.end())
	{
		if (!(*it)->IsStopped())
		{
			(*it)->Call(false);
			++it;
		}
		else
		{
			(*it)->Call(true);
			m_TimerEvents.erase(it++);
		}
	}

	m_LastLoopTime = CTimer::GetCurrentTime();
}

void CTimerEventManager::ChangeTimerState(bool start, const char* id)
{
	if (CTimerEvent* t = GetTimerEvent(id))
		t->SetTimerState(!start);
}

CTimerEvent* CTimerEventManager::GetTimerEvent(const char* id)
{
	for (auto it = m_TimerEvents.begin(); it != m_TimerEvents.end(); ++it)
	{
		if ((*it)->GetID() == id)
			return *it;
	}

	return nullptr;
}


void CTimerEvent::SetTimerState(bool stopped) 
{ 
	m_Stopped = stopped; 

	if (!m_Stopped)
		m_StartTime = m_CurrentTime = CTimer::GetCurrentTime();
}

void CTimerEvent::Call(bool finished)
{
	m_TimeFromPrev = CTimer::GetCurrentTime() - m_CurrentTime;
	m_CurrentTime = CTimer::GetCurrentTime();
	m_TimeFromStart = m_CurrentTime - m_StartTime;
	if (!finished)
		m_Event->HandleEvent();
	else if (m_FinishedEvent)
		m_FinishedEvent->HandleEvent();
}
