#pragma once

#include <memory>
#include <vector>
#include <mutex>

class CLetterModel;
class CTimerEventManager;
class CGameManager;
class CPlayer;

class CWordAnimationManager
{
private:
	
	enum ELetterAnimState {Waiting, InProgress, Finished};

	struct TLetterAnimation
	{
		TLetterAnimation(CLetterModel* letterModel, float distance, float height, size_t letterIdx) : m_LetterModel(letterModel), m_Distance(distance), m_DestHeight(height), m_PlayerLetterIdx(letterIdx) {}

		CLetterModel* m_LetterModel;
		float m_AminationTime = 0.f;
		float m_Distance;
		float m_DestHeight;
		ELetterAnimState m_State = ELetterAnimState::Waiting;
		size_t m_PlayerLetterIdx;
	};

	std::vector<TLetterAnimation> m_LetterAnimations;
	size_t m_CurrentLetterIdx;
	double m_LastAddedLetterTime;
		
	const float m_LetterAddInterval = 800.f; //TODO config
	const float m_LetterAnimTime = 350.f; //TODO config

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	std::shared_ptr<CPlayer> m_Player;

	std::mutex m_AnimListLock;


public:

	CWordAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager) : m_TimerEventManager(timerEventMgr), m_GameManager(gameManager) {}

	void AddWordAnimation(std::wstring word, const std::vector<size_t>& letterIndices, int x, int y, bool horizontal, bool nextPlayerIfFinished = true);
	void AnimateLettersEvent(double& timeFromStart, double& timeFromPrev);
	void AnimationFinished();
};
