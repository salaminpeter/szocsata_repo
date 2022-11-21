#pragma once

#include <list>
#include <string>
#include <mutex>

#include "Event.h"

class CTimerEvent;

class CTimerEvent
{
public:

	CTimerEvent(const char* id) : m_Id(id) {}
	~CTimerEvent();

	template <typename ClassType>
	void SetEvent(ClassType* funcClass, typename CEvent<ClassType, double&, double&>::TFuncPtrType funcPtr)
	{
		m_Event = new CEvent<ClassType, double&, double&>(funcClass, funcPtr, m_TimeFromStart, m_TimeFromPrev);
	}

	template <typename ClassType, typename... ArgTypes>
	void SetFinishedEvent(ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, ArgTypes&&... args)
	{
		m_FinishedEvent = funcPtr ? new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...) : nullptr;
	}

	void Call(bool finished);
	void SetTimerState(bool started, bool stopped, bool paused, bool finished);

	bool IsStopped() { return m_Stopped; }
	bool IsPaused() { return m_Paused; }
	bool IsStarted() { return m_Started; }
	bool IsFinished() { return m_Finished; }
	std::string GetID() {return m_Id;}

private:

	std::string m_Id;
	CEventBase* m_Event = nullptr;
	CEventBase* m_FinishedEvent = nullptr;

	bool m_Stopped = false;
	bool m_Paused = false;
	bool m_Started = false;
	bool m_Finished = false;
	bool m_FinishDone = false;

	double m_StartTime;
	double m_CurrentTime;
	double m_TimeFromStart;
	double m_TimeFromPrev;
	double m_PauseTime;
};

class CTimerEventManager
{
public:
	
	~CTimerEventManager();

	template <typename ClassType, typename... ArgTypes>
	void AddTimerEvent(ClassType* funcClass, typename CEvent<ClassType, double&, double&>::TFuncPtrType eventPtr, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType finishedEventPtr, const char* id)
	{
		if (GetTimerEvent(id))
			return;

		CTimerEvent* Event = new CTimerEvent(id);
		Event->SetEvent(funcClass, eventPtr);
		Event->SetFinishedEvent(funcClass, finishedEventPtr);

		m_TimerEvents.insert(m_TimerEvents.end(), Event);
	}

	void Loop();
	void Reset();
	bool IsTimerRunning(const char* id);

	void StartTimer(const char* id) { ChangeTimerState(true, false, false, false, id); }
	void RestartTimer(const char* id) { StartTimer(id); }
	void RemoveTimer(const char* id) { ChangeTimerState(false, true, false, false, id); }
	void FinishTimer(const char* id) { ChangeTimerState(false, false, false, true, id); }
	void PauseTimer(const char* id) { ChangeTimerState(false, false, true, false, id); }
	void ResumeTimer(const char* id) { ChangeTimerState(true, false, false, false, id); }

private:

	CTimerEvent* GetTimerEvent(const char* id);
	void ChangeTimerState(bool start, bool stop, bool pause, bool finish, const char* id);

private:
	
	const int m_Frequency = 10;
	double m_LastLoopTime = 0;

	std::mutex m_Lock;

	std::list<CTimerEvent*> m_TimerEvents;
};

