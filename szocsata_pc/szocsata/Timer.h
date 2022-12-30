#pragma once

#include <chrono>
#include <map>
#include <string>
#include <ratio>
#include <mutex>

class CTimer
{
public:
	
	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::high_resolution_clock::time_point TTimePoint;
	typedef std::chrono::duration<double, std::milli> TTimeSpan;

	static void Start(const char* id = "")
	{
		m_Timers[id] = Clock::now();
	}

	static double GetTime(const char* id = "")
	{
		TTimePoint Time = Clock::now();
		TTimeSpan TimeSpan = Time - m_Timers[id];
		return TimeSpan.count();
	}

	static double GetCurrentTime()
	{
		const std::lock_guard<std::mutex> lock(m_GetTimeLock);
		TTimeSpan TimeSpan = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch());
		return TimeSpan.count();
	}

private:

	static std::mutex m_GetTimeLock;
	static  std::map<std::string, TTimePoint> m_Timers;
	static std::chrono::high_resolution_clock::time_point m_StartTime;
};