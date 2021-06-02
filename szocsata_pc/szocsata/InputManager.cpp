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
	glm::vec2 v0(x0 - m_Touch0X, y0 - m_Touch0Y);
	glm::vec2 v1(x1 - m_Touch1X, y1 - m_Touch1Y);

	if (glm::length(v0) < 0.1f || glm::length(v1) < 0.1f)
	{
		return;
	}

	float angle = glm::degrees(std::acosf(glm::dot(v0, v1) / (glm::length(v0) * glm::length(v1))));

	if (angle < 30)  //TODO 30 config
	{
		if (m_InputState == Zoom)
			m_GameManager->HandleZoomEndEvent();

		m_InputState = Drag;
	}
	else
		m_InputState = Zoom;

	if (m_InputState == Zoom) {
		float Dist = std::sqrtf((x0 - x1) * (x0 - x1) +  (y0  - y1) * (y0 - y1));

		if (std::fabs(Dist - m_TouchDist) < 0.01)
			return;

		m_GameManager->HandleZoomEvent((Dist - m_TouchDist) / 60.f, m_TouchCenterX, m_TouchCenterY);
		m_TouchDist = Dist;
	}
	else if (m_InputState == Drag)
	{
		float dist = glm::length((v0 + v1) / 2.f);
		m_GameManager->HandleMultyDragEvent(m_Touch0X, m_Touch0Y, x0, y0);
	}

	m_Touch0X = x0;
	m_Touch0Y = y0;
	m_Touch1X = x1;
	m_Touch1Y = y1;
}


void CInputManager::HandleReleaseEvent(int x, int y)
{
	m_GameManager->HandleReleaseEvent(x, y);
	m_InputState = None;
}

void CInputManager::HandleDragEvent(int x, int y)
{
	m_GameManager->HandleDragEvent(x, y);
}


