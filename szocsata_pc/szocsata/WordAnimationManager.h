#pragma once

#include <memory>
#include <vector>
#include <mutex>

class CLetterModel;
class CTimerEventManager;
class CGameManager;
class CPlayer;
class CUIPlayerLetters;
class CBinaryBoolList;

class CWordAnimationManager
{
public:

	~CWordAnimationManager();

private:
	
	enum ELetterAnimState {Waiting, InProgress, Finished};

	struct TLetterAnimation
	{
		TLetterAnimation(CLetterModel* letterModel, float distance, float height, size_t uiLetterIdx, int boardX, int boardY) : m_LetterModel(letterModel), m_Distance(distance), m_DestHeight(height), m_UILetterIdx(uiLetterIdx), m_BoardX(boardX), m_BoardY(boardY) {}

		CLetterModel* m_LetterModel;
		float m_AminationTime = 0.f;
		float m_Distance;
		float m_DestHeight;
		int m_BoardX;
		int m_BoardY;
		size_t m_UILetterIdx;
		ELetterAnimState m_State = ELetterAnimState::Waiting;
	};

	std::vector<TLetterAnimation> m_LetterAnimations;
	size_t m_CurrentLetterIdx;
	double m_LastAddedLetterTime;
		
	const float m_LetterAddInterval = 800.f; //TODO config
	const float m_LetterAnimTime = 350.f; //TODO config

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	CUIPlayerLetters* m_UIPlayerLetters;

	std::mutex m_AnimListLock;


public:

	CWordAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager) : m_TimerEventManager(timerEventMgr), m_GameManager(gameManager) {}

	void AddWordAnimation(std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool nextPlayerIfFinished = true);
	void AnimateLettersEvent(double& timeFromStart, double& timeFromPrev);
	void AnimationFinished();
	void Reset();
};
