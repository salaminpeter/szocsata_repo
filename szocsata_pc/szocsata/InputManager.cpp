#include "stdafx.h"
#include "InputManager.h"
#include "GameManager.h"


void CInputManager::HandleTouchEvent(int x, int y, bool onBoardView)
{
	m_GameManager->HandleToucheEvent(x, y, onBoardView);
}

void CInputManager::HandleZoomEvent(float dist, float origoX, float origoY)
{
	m_GameManager->HandleZoomEvent(dist, origoX, origoY);
}

void CInputManager::HandleMultyTouchStart(float x0, float y0, float x1, float y1)
{
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
	m_GameManager->HandleZoomEndEvent();
}

void CInputManager::HandleMultyTouch(float x0, float y0, float x1, float y1)
{
	m_Touch0X = x0;
	m_Touch0Y = y0;
	m_Touch1X = x1;
	m_Touch1Y = y1;

	float Dist = std::sqrtf((m_Touch0X - m_Touch1X) * (m_Touch0X - m_Touch1X) + (m_Touch0Y - m_Touch1Y) * (m_Touch0Y - m_Touch1Y));

	if (std::fabs(Dist - m_TouchDist) < 0.01)
		return;

	m_GameManager->HandleZoomEvent((Dist - m_TouchDist) / 60.f, m_TouchCenterX, m_TouchCenterY);
	m_TouchDist = Dist;
}


void CInputManager::HandleReleaseEvent(int x, int y)
{
	m_GameManager->HandleReleaseEvent(x, y);
}

void CInputManager::HandleDragEvent(int x, int y)
{
	m_GameManager->HandleDragEvent(x, y);
}


