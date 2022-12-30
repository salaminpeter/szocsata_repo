#include "stdafx.h"
#include "Timer.h"


std::chrono::high_resolution_clock::time_point CTimer::m_StartTime;
std::map<std::string, CTimer::TTimePoint> CTimer::m_Timers;
std::mutex CTimer::m_GetTimeLock;