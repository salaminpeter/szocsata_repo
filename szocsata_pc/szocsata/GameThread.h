#pragma once

#include <thread>
#include <string>
#include <mutex>

class CGameManager;

class CGameThread
{
public:

	CGameThread(CGameManager* gm) : m_GameManager(gm) {}
	~CGameThread() {delete m_Thread;}

	void Start();
	void AddTaskToExecute(const char* id);

private:

	CGameManager* m_GameManager;
	std::thread* m_Thread;
	std::string m_TaskIdToExecute;
	std::mutex m_TaskMutex;
};