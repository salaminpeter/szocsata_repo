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

void CTaskManager::Reset()
{
	PauseThread();

	{
        const std::lock_guard<std::recursive_mutex> lock(m_Lock);
        m_TaskList.clear();
    }

    ResumeThread();
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
	while (true)
	{
		if (m_StopTaskThread)
			return;

		if (m_PauseTaskThread)
			continue;

		/*
		if (CTimer::GetCurrentTime() - m_LastLoopTime < m_Frequency)
			continue;
		 */
		static int ii = 0;

		if (ii == 50)

		{
			ii = 0;
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
					if ((*it)->m_ID == "generate_models_task")
						int i = 0;
					if ((*it)->m_Task)
						m_GameManager->ExecuteTaskOnThread((*it)->m_ID.c_str(), (*it)->m_RunOnThread);
				}
			}
		}

		ii++;

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





