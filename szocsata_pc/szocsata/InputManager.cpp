#include "stdafx.h"
#include "InputManager.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "Config.h"

CInputManager::CInputManager(CGameManager* gameManager) :
m_GameManager(gameManager)
{
	m_GameManager->GetTimerEventManager()->AddTimerEvent(this, &CInputManager::CheckDoubleClickEvent, nullptr, "timer_event_double_click");
}

void CInputManager::HandleTouchEvent(int x, int y)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);

	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	//first touch event 
	if (!m_FirstTouch && !m_SecondTouch)
	{ 
		m_FirstTouch = true;
		m_Dragged = true;
		m_DoubleClickTimePassed = false;
		m_DoubleClickHandled = false;
	}

	//second touch event
	else if (m_FirstTouch && !m_SecondTouch)
	{ 
		m_SecondTouch = true;
		m_FirstTouch = false;
	}
	
	bool OnBoardView = m_GameManager->GameScreenActive() && x <= WindowHeigth;
	m_GameManager->SetLastTouchOnBoardView(OnBoardView);
	m_GameManager->SetLastTouchPos(x, WindowHeigth - y);

	//dupla clicket csak board viewra nezunk
	if (m_FirstTouch)
	{
		m_Touch0X = x;
		m_Touch0Y = y;
		m_GameManager->GetTimerEventManager()->StartTimer("timer_event_double_click");
		return;
	}
}

void CInputManager::HandleZoomEvent(float dist, float origoX, float origoY)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);
	m_GameManager->HandleZoomEvent(dist, origoX, origoY);
}

void CInputManager::HandleMultyTouchStart(float x0, float y0, float x1, float y1)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);
	m_Touch0X = x0;
	m_Touch0Y = y0;
	m_Touch1X = x1;
	m_Touch1Y = y1;
	m_TouchCenterX = (m_Touch0X + m_Touch1X) / 2.f;
	m_TouchCenterY = (m_Touch0Y + m_Touch1Y) / 2.f;
	m_TouchDist = std::sqrtf((m_Touch0X - m_Touch1X) * (m_Touch0X - m_Touch1X) + (m_Touch0Y - m_Touch1Y) * (m_Touch0Y - m_Touch1Y));
}

void CInputManager::HandleMultyTouchEnd()
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);
	m_GameManager->HandleZoomEndEvent();
}

void CInputManager::HandleMultyTouch(float x0, float y0, float x1, float y1)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);
	glm::vec2 v0(x0 - m_Touch0X, y0 - m_Touch0Y);
	glm::vec2 v1(x1 - m_Touch1X, y1 - m_Touch1Y);

	if (glm::length(v0) < 0.1f || glm::length(v1) < 0.1f)
	{
		return;
	}

	float Dist = std::sqrtf((x0 - x1) * (x0 - x1) +  (y0  - y1) * (y0 - y1));

	if (std::fabs(Dist - m_TouchDist) < 0.01)
		return;

	float angle = glm::degrees(std::acosf(glm::dot(v0, v1) / (glm::length(v0) * glm::length(v1))));

    bool Drag = (angle < 60);
    bool Zoom = (angle > 30);

    if (Drag)
    {
        m_GameManager->HandleMultyDragEvent(m_Touch0X, m_Touch0Y, x0, y0);
        m_GameManager->HandleZoomEndEvent();
    }

    if (Zoom && Drag)
        m_GameManager->HandleZoomEvent((Dist - m_TouchDist) / 60.f);
    else if (Zoom && !Drag)
        m_GameManager->HandleZoomEvent((Dist - m_TouchDist) / 60.f, m_TouchCenterX, m_TouchCenterY);

	m_Touch0X = x0;
	m_Touch0Y = y0;
	m_Touch1X = x1;
	m_Touch1Y = y1;
	m_TouchDist = Dist;
}


void CInputManager::HandleReleaseEvent(int x, int y)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);

	//dragging stop, CheckDoubleClickEvent already finished, have to handle release event here
	if (m_DoubleClickTimePassed && m_Dragged)
	{
		m_FirstTouch = false;
		m_SecondTouch = false; 
		m_Dragged = false;
		m_ReleaseTouchHappened = false;
		m_GameManager->HandleReleaseEvent(x, y);
		return;
	}

	//masodik klikk felengedese
	if (!m_FirstTouch && !m_SecondTouch)
	{
		if (m_DoubleClickTimePassed)
		{
			if (!m_DoubleClickHandled)
			{ 
				m_GameManager->HandleToucheEvent(m_Touch1X, m_Touch1Y);
				m_GameManager->HandleReleaseEvent(m_Touch1X, m_Touch1Y);
				m_SecondReleaseTouchHappened = false;
			}
		}
		else
			m_SecondReleaseTouchHappened = true;

	}
		
	//waiting for duobleclick
	if (m_FirstTouch)
	{
		m_Dragged = false;
		m_ReleaseTouchHappened = true;
		m_Touch1X = x;
		m_Touch1Y = y;
	}
}

void CInputManager::HandleDragEvent(int x, int y)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);
	
	if (m_Dragged && m_DoubleClickTimePassed)
		m_GameManager->HandleDragEvent(x, y);
}

void CInputManager::CheckDoubleClickEvent(double& timeFromStart, double& timeFromPrev)
{
	const std::lock_guard<std::recursive_mutex> lock(m_InputLock);

	if (timeFromStart >= 250)  //TODO config
	{
		m_GameManager->GetTimerEventManager()->FinishTimer("timer_event_double_click");

		//2x is le lett nyomva adott idon belul a gomb
		bool IsDoubleClick = m_SecondTouch;
		bool CantHandleDoubleClick = false;

		if (IsDoubleClick)
			m_DoubleClickHandled = m_GameManager->HandleDoubleClickEvent(m_Touch0X, m_Touch0Y);

		CantHandleDoubleClick = !m_DoubleClickHandled;

		if (!IsDoubleClick)
			CantHandleDoubleClick = true;

		if (CantHandleDoubleClick)
		{
			if (m_ReleaseTouchHappened || m_SecondReleaseTouchHappened)
			{
				m_GameManager->HandleToucheEvent(m_Touch0X, m_Touch0Y);
				m_GameManager->HandleReleaseEvent(m_Touch0X, m_Touch0Y);
			}
			if (m_SecondReleaseTouchHappened)
			{
				m_GameManager->HandleToucheEvent(m_Touch1X, m_Touch1Y);
				m_GameManager->HandleReleaseEvent(m_Touch1X, m_Touch1Y);
			}
		}

		//letelt a double click ido, es kozben a gombot vegig lenyomva tartottuk
		if (m_FirstTouch && !m_SecondTouch && !m_ReleaseTouchHappened)
			m_GameManager->HandleToucheEvent(m_Touch0X, m_Touch0Y);

		if (!(m_FirstTouch && m_Dragged))
			m_FirstTouch = false;

		m_SecondTouch = m_ReleaseTouchHappened = m_SecondReleaseTouchHappened = false;
		m_DoubleClickTimePassed = true;
	}
}
