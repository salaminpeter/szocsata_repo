#include "stdafx.h"
#include "GameThread.h"
#include "GameManager.h"

void CGameThread::Start()
{
	m_Thread = new std::thread(&CGameManager::GameLoop, m_GameManager);
}

void CGameThread::AddTaskToExecute(const char* id)
{
	m_GameManager->StartTask(id);
}
