#include "stdafx.h"
#include "TimerEventManager.h"
#include "Timer.h"

#include <algorithm>
#include <mutex>

namespace Logger {
#ifdef WIN32
    void Log(const char *msg) {}
#else
	void Log(const char *msg);
#endif
}

CTimerEventManager::~CTimerEventManager()
{
	Reset();
}

void CTimerEventManager::Reset()
{
	auto it = m_TimerEvents.begin();

	while (it != m_TimerEvents.end())
		(*it++)->SetTimerState(false, true);
}


void CTimerEventManager::Loop()
{
	if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
        return;

	const std::lock_guard<std::mutex> lock(m_Lock);

	auto it = m_TimerEvents.begin();
	
	while (it != m_TimerEvents.end())
	{
		if (!(*it)->IsStopped() && !(*it)->IsPaused() && (*it)->IsStarted())
		{
			(*it)->Call(false);
			++it;
			continue;
		}
		else if ((*it)->IsStopped())
		{
			(*it)->Call(true);
			m_TimerEvents.erase(it++);
			continue;
		}

		++it;
	}

	m_LastLoopTime = CTimer::GetCurrentTime();
}

void CTimerEventManager::ChangeTimerState(bool start, bool stop, bool pause, const char* id)
{
	if (CTimerEvent* t = GetTimerEvent(id))
		t->SetTimerState(start, stop, pause);
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


CTimerEvent::~CTimerEvent() 
{ 
	delete m_Event; 
	delete m_FinishedEvent; 
}


void CTimerEvent::SetTimerState(bool started, bool stopped, bool paused) 
{ 
	//pause timer
	if (!m_Paused && paused)
		m_Paused = true;

	//resume timer
	if (m_Paused && !paused)
	{
		m_Paused = false;
		m_CurrentTime = CTimer::GetCurrentTime();
		m_StartTime = m_CurrentTime - m_TimeFromStart;
		return;
	}

	if (m_Paused)
		return;

	m_Stopped = stopped;

	if (m_Stopped)
		return;

	m_Started = started;

	if (m_Started)
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
