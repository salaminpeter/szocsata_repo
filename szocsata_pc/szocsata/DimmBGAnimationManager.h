#pragma once

class CTimerEventManager;
class CGameManager;

class CDimmBGAnimationManager
{
public:

	CDimmBGAnimationManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : m_TimerEventManager(timerEventManager), m_GameManager(gameManager) {}

	void StartAnimation(bool fadiIn = true);

private:

	void AnimateBackground(double& timeFromStart, double& timeFromPrev);
	void AnimFinishedEvent();

private:

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;

	bool m_FadeIn;
};
