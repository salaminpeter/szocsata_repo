#pragma once

#include <vector>

class CUIElement;
class CTimerEventManager;
class CGameManager;


class CPlayerLetterAnimationManager
{
private:

	struct TPlayerLetterAnimation
	{
		TPlayerLetterAnimation(CUIElement* playerLetter, float destScale) : m_PlayerLetter(playerLetter), m_DestScale(destScale) {};

		CUIElement* m_PlayerLetter;
		float m_DestScale;
	};

public:

	CPlayerLetterAnimationManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : m_TimerEventManager(timerEventManager), m_GameManager(gameManager) {}

	void StartAnimations();
	void AddAnimation(CUIElement* playerLEtter, float destScale);

	bool Finished() {return (m_PlayerLetterAnimations.size() == 0);}

private:

	void AnimatePlayerLetter(double& timeFromStart, double& timeFromPrev);
	void AnimFinishedEvent();

private:

	std::vector<TPlayerLetterAnimation> m_PlayerLetterAnimations;
	size_t m_CurrentLetterIdx;
	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
};

