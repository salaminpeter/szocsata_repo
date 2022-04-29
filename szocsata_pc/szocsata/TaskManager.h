#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "Event.h"

class CEventBase;
class CTask;
class CGameManager;

class CTask
{
public: //TODO

    enum ERunSource {RenderThread, GameThread, CurrentThread};

    CEventBase* m_Task = nullptr;
    std::list<std::shared_ptr<const CTask>> m_DependencyList;
    std::string m_ID;
    bool m_TaskStarted = false;
    bool m_TaskFinished = false;
    bool m_TaskStopped = true;
    ERunSource m_RunOnThread ;//= CTaskManager::CurrentThread;

    CTask(const char* id, ERunSource thread) : m_ID(id), m_RunOnThread(thread) {}
	~CTask() {delete m_Task;}

    void AddDependencie(std::shared_ptr<CTask> dep)
    {
        m_DependencyList.insert(m_DependencyList.begin(), dep);
    }

    template <typename ClassType, typename... ArgTypes>
    void SetTask(ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, ArgTypes&&... args)
    {
    	if (funcPtr)
        	m_Task = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
    	else
    		m_Task = nullptr;
    }

    bool DependenciesResolved();
    void Start();
};


class CTaskManager
{
public:
	
	CTaskManager(CGameManager* gm) : m_GameManager(gm)
	{
		m_Thread = new std::thread(&CTaskManager::TaskLoop, this);
	}

	~CTaskManager()
	{
		//delete m_Thread; //TODO nem lehet egyszeruen deletelni, joinolni kell hozaa
	}


	template <typename ClassType, typename... ArgTypes>
	std::shared_ptr<CTask> AddTask(ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, const char* id, CTask::ERunSource runThread, ArgTypes&&... args)
	{
		const std::lock_guard<std::recursive_mutex> lock(m_Lock);

		std::shared_ptr<CTask> FoundTask = GetTask(id);
		if (FoundTask)
		{
			FoundTask->m_TaskFinished = false;
			FoundTask->m_TaskStarted = false;
			return FoundTask;
		}

		std::shared_ptr<CTask> Task = std::make_shared<CTask>(id, runThread);
		Task->SetTask(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
		m_TaskList.insert(m_TaskList.begin(), Task);

		return Task;
	}
	
	void AddDependencie(const char* taskId, const char* depId);
	void SetTaskFinished(const char* taskId);
	void TaskLoop();
	void StartTask(const char* id);
	void Reset();

	void StopThread() { m_StopTaskThread = true; }
	void PauseThread() { m_PauseTaskThread = true; }
	void ResumeThread() { m_PauseTaskThread = false; }

private:
	
	const int m_Frequency = 10;
	std::list<std::shared_ptr<CTask>> m_TaskList;
	double m_LastLoopTime = 0;
	std::recursive_mutex m_Lock;
	std::thread* m_Thread;
	CGameManager* m_GameManager;
	bool m_StopTaskThread = false;
	bool m_PauseTaskThread = false;


private:

	std::shared_ptr<CTask> GetTask(const char* id);
	void FreeTask(const char* taskId);
};

