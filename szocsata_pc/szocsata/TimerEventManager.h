#pragma once

#include <list>
#include <string>

#include "Event.h"

class CTimerEvent;

class CTimerEvent
{
public:

	CTimerEvent(const char* id) : m_Id(id) {}

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
	void SetTimerState(bool stopped);

	bool IsStopped() {return m_Stopped;}
	std::string GetID() {return m_Id;}

private:

	std::string m_Id;
	CEventBase* m_Event = nullptr;
	CEventBase* m_FinishedEvent = nullptr;
	bool m_Stopped = true;
	double m_StartTime;
	double m_CurrentTime;
	double m_TimeFromStart;
	double m_TimeFromPrev;
};

class CTimerEventManager
{
public:
	
	template <typename ClassType, typename... ArgTypes>
	void AddTimerEvent(ClassType* funcClass, typename CEvent<ClassType, double&, double&>::TFuncPtrType eventPtr, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType finishedEventPtr, const char* id)
	{
		CTimerEvent* Event = new CTimerEvent(id);
		Event->SetEvent(funcClass, eventPtr);
		Event->SetFinishedEvent(funcClass, finishedEventPtr);

		m_TimerEvents.insert(m_TimerEvents.end(), Event);
	}

	void Loop();

	void StartTimer(const char* id) { ChangeTimerState(true, id); }
	void StopTimer(const char* id) { ChangeTimerState(false, id); }

private:

	CTimerEvent* GetTimerEvent(const char* id);
	void ChangeTimerState(bool start, const char*id);

private:
	
	const int m_Frequency = 10;
	double m_LastLoopTime = 0;

	std::list<CTimerEvent*> m_TimerEvents;
};

