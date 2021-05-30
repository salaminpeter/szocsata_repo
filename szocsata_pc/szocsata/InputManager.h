#pragma once

class CGameManager;

class CInputManager
{
public:
	
	CInputManager(CGameManager* gameManager) : m_GameManager(gameManager) {}

	void HandleTouchEvent(int x, int y, bool onBoardView);
	void HandleReleaseEvent(int x, int y);
	void HandleDragEvent(int x, int y);
	void HandleZoomEvent(float dist, float origoX, float origoY);
	void HandleMultyTouchStart(float x0, float y0, float x1, float y1);
	void HandleMultyTouch(float x0, float y0, float x1, float y1);
	void HandleMultyTouchEnd();

private:
	
	float m_Touch0X;
	float m_Touch0Y;
	float m_Touch1X;
	float m_Touch1Y;
	float m_TouchCenterX;
	float m_TouchCenterY;
	float m_TouchDist;

	CGameManager* m_GameManager;
};