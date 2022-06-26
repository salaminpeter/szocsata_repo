#pragma once

#include <mutex>

class CGameManager;

class CInputManager
{
public:

	CInputManager(CGameManager* gameManager) : 
		m_GameManager(gameManager)
	{}

	void HandleTouchEvent(int x, int y);
	void HandleReleaseEvent(int x, int y);
	void HandleDragEvent(int x, int y);
	void HandleZoomEvent(float dist, float origoX, float origoY);
	void HandleMultyTouchStart(float x0, float y0, float x1, float y1);
	void HandleMultyTouch(float x0, float y0, float x1, float y1);
	void HandleMultyTouchEnd();

	void CheckDoubleClickEvent(double& timeFromStart, double& timeFromPrev);


private:
	
	float m_Touch0X;
	float m_Touch0Y;
	float m_Touch1X;
	float m_Touch1Y;
	float m_TouchCenterX;
	float m_TouchCenterY;
	float m_TouchDist;

	bool m_FirstTouch = false;
	bool m_SecondTouch = false;
	bool m_ReleaseTouchHappened = false;
	bool m_Dragged = false;
	bool m_DoubleClickTimePassed = false;

	CGameManager* m_GameManager;
	std::mutex m_InputLock;
};