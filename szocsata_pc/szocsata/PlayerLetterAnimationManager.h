#pragma once

#include <vector>

class CUIElement;
class CTimerEventManager;


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

	CPlayerLetterAnimationManager(CTimerEventManager* timerEventManager) : m_TimerEventManager(timerEventManager) {}

	void StartAnimations();
	void AddAnimation(CUIElement* playerLEtter, float destScale);

private:

	void AnimatePlayerLetter(double& timeFromStart, double& timeFromPrev);

private:

	std::vector<TPlayerLetterAnimation> m_PlayerLetterAnimations;
	size_t m_CurrentLetterIdx;
	CTimerEventManager* m_TimerEventManager;
};

