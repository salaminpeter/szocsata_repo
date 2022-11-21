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

bool CTimerEventManager::IsTimerRunning(const char* id)
{
	if (CTimerEvent* t = GetTimerEvent(id))
		return (t->IsStarted() && !t->IsPaused());

	return false;
}

void CTimerEventManager::Reset()
{
	auto it = m_TimerEvents.begin();

	while (it != m_TimerEvents.end())
		(*it++)->SetTimerState(false, false, true, false);
}


void CTimerEventManager::Loop()
{
	if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
        return;

	const std::lock_guard<std::mutex> lock(m_Lock);

	auto it = m_TimerEvents.begin();
	
	while (it != m_TimerEvents.end())
	{
		//timer task is started, its running
		if (!(*it)->IsFinished() && !(*it)->IsStopped() && !(*it)->IsPaused() && (*it)->IsStarted())
			(*it)->Call(false);

		//timer task is stopped, it is removed
		else if ((*it)->IsStopped()) //TODO nem kell ez a finsihed event alig van valahol hasznalva
		{
			it = m_TimerEvents.erase(it);
			continue;
		}

		//timer task is finished
		else if ((*it)->IsFinished())
			(*it)->Call(true);

		++it;
	}

	m_LastLoopTime = CTimer::GetCurrentTime();
}

void CTimerEventManager::ChangeTimerState(bool start, bool stop, bool pause, bool finish, const char* id)
{
	if (CTimerEvent* t = GetTimerEvent(id))
		t->SetTimerState(start, stop, pause, finish);
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


void CTimerEvent::SetTimerState(bool started, bool stopped, bool paused, bool finished)
{
	//finish timer
	if (finished)
	{
		m_Finished = true;
		m_FinishDone = false;
		m_Started = m_Stopped = m_Paused = false;
		return;
	}
	else
		m_Finished = false;

	//pause timer
	if (!m_Paused && paused)
		m_Paused = true;

	//resume timer
	if (m_Paused && !paused && !started)
	{
		m_Paused = false;
		m_CurrentTime = CTimer::GetCurrentTime();
		m_StartTime = m_CurrentTime - m_TimeFromStart;
		m_Started = true;
		return;
	}

	//start / restart timer
	if ((m_Paused || m_Finished) && started)
	{
		m_Paused = false;
		m_Finished = false;
		m_StartTime = m_CurrentTime = CTimer::GetCurrentTime();
		m_Started = true;
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
	else if (m_FinishedEvent && !m_FinishDone)
	{
		m_FinishDone = true;
		m_FinishedEvent->HandleEvent();
	}
}
