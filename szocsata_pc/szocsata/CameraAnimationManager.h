#pragma once


class CTimerEventManager;
class CGameManager;


class CCameraAnimationManager
{
public:

	CCameraAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager);

	void StartFitToScreenAnimation();
	void AnimateCamera(double& timeFromStart, double& timeFromPrev);

private:

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;

	float m_DestTilt;
	float m_CurrTilt;
	float m_DestRotation;
	float m_CurrRotation;
	float m_DestZoom;
	float m_CurrZoom;
};