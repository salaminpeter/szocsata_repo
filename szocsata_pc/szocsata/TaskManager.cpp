#include "stdafx.h"
#include "TaskManager.h"
#include "Event.h"
#include "Timer.h"
#include "GameManager.h"

#include <chrono>

namespace Logger {
	void Log(const char *msg);
}

void CTaskManager::FreeTask(const char* taskId)
{
	for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
	{
		if ((*it)->m_ID == taskId)
		{
			for (auto it1 = (*it)->m_DependencyList.begin(); it1 != (*it)->m_DependencyList.end(); ++it1)
				(*it1).reset();

			(*it).reset();

			if (*it == nullptr)
				m_TaskList.remove(nullptr);

			return;
		}
	}
}

std::shared_ptr<CTask> CTaskManager::GetTask(const char* id)
{
    for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
	{
		if ((*it)->m_ID == id)
			return (*it);
	}

	return nullptr;
}

void CTaskManager::AddDependencie(const char* taskId, const char* depId)
{
	const std::lock_guard<std::mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(taskId);
	std::shared_ptr<CTask> Dep = GetTask(depId);

	if (!Task || !Dep)
		return;

	Task->AddDependencie(Dep);
}

void CTaskManager::FinishTask(const char *taskId)
{
    const std::lock_guard<std::mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(taskId);

    if (!Task)
        return;

    Task->m_TaskFinished = true;
    FreeTask(taskId);
}

void CTaskManager::SetTaskFinished(const char* taskId)
{
	std::thread* FinishTaskThread = new std::thread(&CTaskManager::FinishTask, this, taskId); //TODO nagyon ocsmany leak!!!!!
}

void CTaskManager::Reset()
{
	const std::lock_guard<std::mutex> lock(m_Lock);
    m_TaskList.clear();
 }

void CTaskManager::StartTask(const char* id, bool onCurrentThread)
{
	if (!onCurrentThread)
		m_Lock.lock();

	std::shared_ptr<CTask> Task = GetTask(id);

	if (!Task)
	{
		if (!onCurrentThread)
			m_Lock.unlock();

		return;
	}

	Task->Start();

	if (!onCurrentThread)
		m_Lock.unlock();
}

void CTaskManager::TaskLoop()
{
	while (true)
	{
		if (m_StopTaskThread)
			return;

		if (m_PauseTaskThread)
			continue;


		if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
			continue;

		{
            const std::lock_guard<std::mutex> lock(m_Lock);

			for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
			{
				if ((*it)->m_TaskFinished || (*it)->m_TaskStopped)
					continue;

				if (!(*it)->m_TaskStarted)
				{
					if (!(*it)->DependenciesResolved() || (*it)->m_TaskStarted)
						continue;

					(*it)->m_TaskStarted = true;
					if ((*it)->m_Task)
						m_GameManager->ExecuteTaskOnThread((*it)->m_ID.c_str(), (*it)->m_RunOnThread);
				}
			}
		}

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





