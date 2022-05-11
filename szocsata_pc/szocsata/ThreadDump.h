#pragma once

#include <thread>
#include <atomic>
#include <list>

class CThreadDump
{
public:

	struct TThreadStatus
	{
		std::thread* m_Thread;
		std::atomic<bool>* m_ThreadFinished;

		TThreadStatus(std::thread* t, std::atomic<bool>* finished) : m_Thread(t), m_ThreadFinished(finished) {}
	};

	~CThreadDump() {DeleteStoppedThreads(true);}

	void AddThread(std::thread* t, std::atomic<bool>* flag);
	void DeleteStoppedThreads(bool waitForFinish = false);

private:

	std::list<TThreadStatus> m_Threads;
};
