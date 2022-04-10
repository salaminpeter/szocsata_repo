#include "stdafx.h"
#include "TaskManager.h"
#include "Event.h"
#include "Timer.h"
#include "GameManager.h"


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

void CTaskManager::SetTaskFinished(const char* taskId)
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(taskId);

	if (!Task)
		return;

	Task->m_TaskFinished = true;
}

void CTaskManager::StartTask(const char* id, bool runDirectly) 
{
	const std::lock_guard<std::recursive_mutex> lock(m_Lock);

	std::shared_ptr<CTask> Task = GetTask(id);

	if (!Task)
		return;

	if (runDirectly)
		Task->Start();
	else
	{
#ifdef WIN32
		m_GameManager->m_TaskToStartID = Task->m_ID;
#else
		m_GameManager->RunTaskOnRenderThread(Task->m_ID.c_str());
#endif
	}
}

void CTaskManager::TaskLoop()
{
	while (true)
	{
		if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
			continue;

		{
			const std::lock_guard<std::recursive_mutex> lock(m_Lock);

			for (auto it = m_TaskList.begin(); it != m_TaskList.end(); ++it)
			{
				if ((*it)->m_TaskFinished || (*it)->m_TaskStopped)
					continue;

				if (!(*it)->m_TaskStarted)
				{
					if (!(*it)->DependenciesResolved() || (*it)->m_TaskStarted)
						continue;

					(*it)->m_TaskStarted = true;
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





