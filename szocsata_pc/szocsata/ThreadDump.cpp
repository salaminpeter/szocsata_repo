#include "stdafx.h"
#include "ThreadDump.h"

void CThreadDump::AddThread(std::thread* t, std::atomic<bool>* flag)
{ 
	m_Threads.emplace_back(t, flag); 
}

void CThreadDump::DeleteStoppedThreads(bool waitForFinish)
{
	for (auto it = m_Threads.begin(); it != m_Threads.end(); ++it)
	{
		if (*it->m_ThreadFinished || waitForFinish)
		{
			it->m_Thread->join();
			delete it->m_Thread;
			delete it->m_ThreadFinished;
			it->m_Thread = nullptr;
			it->m_ThreadFinished = nullptr;
		}
	}

	auto ListEnd = std::remove_if(m_Threads.begin(), m_Threads.end(), [](const TThreadStatus& t){return (!t.m_Thread && !t.m_ThreadFinished);});
	m_Threads.erase(ListEnd, m_Threads.end());
}

