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
		TPlayerLetterAnimation(CUIElement* playerLetter, float destScale, float startX, float startY, float destX, float destY) : 
			m_PlayerLetter(playerLetter), 
			m_DestScale(destScale),
			m_StartX(startX),
			m_StartY(startY),
			m_DestX(destX),
			m_DestY(destY)
		{};

		CUIElement* m_PlayerLetter;
		float m_DestScale;
		float m_StartX;
		float m_StartY;
		float m_DestX;
		float m_DestY;
	};

public:

	CPlayerLetterAnimationManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : m_TimerEventManager(timerEventManager), m_GameManager(gameManager) {}

	void StartAnimations();
	void AddAnimation(CUIElement* playerLEtter, float destScale, float startX, float startY, float destX, float destY);

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

