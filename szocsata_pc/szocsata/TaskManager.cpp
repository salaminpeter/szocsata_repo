#include "stdafx.h"
#include "TaskManager.h"
#include "Event.h"
#include "Timer.h"
#include "GameManager.h"
#include "ThreadDump.h"

#include <chrono>
#include <future>

namespace Logger {
	void Log(const char *msg);
}

CTaskManager::CTaskManager(CGameManager* gm) : m_GameManager(gm)
{
	m_Thread = new std::thread(&CTaskManager::TaskLoop, this);
	m_ThreadDump = std::make_unique<CThreadDump>();
}


void CTaskManager::FreeTask(const char* taskId)
{
	for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
	{
		if ((*it)->m_ID == taskId)
		{
			for (auto it1 = (*it)->m_DependencyList.begin(); it1 != (*it)->m_DependencyList.end(); ++it1)
				(*it1).reset();

			(*it)->m_DependencyList.clear();
			(*it).reset();

			size_t PrevSize = m_TaskList.size();

			if (*it == nullptr)
				m_TaskList.remove(nullptr);

			if (m_TaskList.size() == 0 && PrevSize != 0)
				m_TasksDone = true;

			return;
		}
	}
}

std::shared_ptr<CTask> CTaskManager::GetTask(const char* id)
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

    for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
	{
		if ((*it)->m_ID == id)
			return (*it);
	}

	return nullptr;
}

void CTaskManager::AddDependencie(const char* taskId, const char* depId)
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(taskId);
	std::shared_ptr<CTask> Dep = GetTask(depId);

	if (!Task || !Dep)
		return;

	Task->AddDependencie(Dep);
}

#include <chrono>
#include <thread>

bool CTaskManager::StartedTasksFinished()
{
	for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
	{
		if ((*it)->m_Task && (*it)->m_TaskStarted && !(*it)->m_TaskFinished)
			return false;
	}

	return true;
}


void CTaskManager::WaitForTasksToFinish()
{
	while (!m_TasksDone)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}

	m_TasksDone = false;
}

void CTaskManager::FinishTask(const char *taskId, std::atomic<bool>* flag)
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(taskId);

	if (Task) 
		Task->m_TaskFinished = true;

	FreeTask(taskId);
	*flag = true;
}

void CTaskManager::SetTaskFinished(const char* taskId)
{
	const std::lock_guard<std::mutex> lock(m_FinishLock);

	std::atomic<bool>* IsFinished = new std::atomic<bool>(false);
	std::thread* FinishTaskThread = new std::thread(&CTaskManager::FinishTask, this, taskId, IsFinished); //TODO nagyon ocsmany leak!!!!!
	m_ThreadDump->AddThread(FinishTaskThread, IsFinished);
}

void CTaskManager::Reset()
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);
    m_TaskList.clear();
}

void CTaskManager::ActivateTask(const char* id)
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(id);

	if (!Task)
		return;

	Task->m_TaskStopped = false;
}

void CTaskManager::StartTask(const char* id)
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(id);

	if (!Task)
		return;

	Task->Start();
}

void CTaskManager::TaskLoop()
{
	m_TaskThreadStopped = false;

	while (true)
	{
		if (m_StopTaskThread && StartedTasksFinished())
		{
			m_TaskThreadStopped = true;
			return;
		}

		if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
			continue;

		{
			const std::lock_guard<std::recursive_mutex> lock(m_Lock);

			for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
			{
				if ((*it)->m_TaskFinished || (*it)->m_TaskStopped)
					continue;

				if (!m_StopTaskThread && !(*it)->m_TaskStarted)
				{
					if (!(*it)->DependenciesResolved())
						continue;

					(*it)->m_TaskStarted = true;
					if ((*it)->m_Task)
						m_GameManager->ExecuteTaskOnThread((*it)->m_ID.c_str(), (*it)->m_RunOnThread);
				}
			}
		}

		if (m_ThreadDump)
			m_ThreadDump->DeleteStoppedThreads();

		m_LastLoopTime = CTimer::GetCurrentTime();
	}
}

bool CTask::DependenciesResolved()
{
	for (auto it = m_DependencyList.begin(); it != m_DependencyList.end(); ++it)
	{
		if (!(*it)->m_TaskFinished)
			return false;
	}

	return true;
}

void CTask::Start()
{
	if (!DependenciesResolved())
		return;

	m_TaskStarted = true;
	m_Task->HandleEvent();
}





