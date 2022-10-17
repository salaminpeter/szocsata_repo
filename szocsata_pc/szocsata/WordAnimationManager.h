#pragma once

#include <memory>
#include <list>
#include <vector>
#include <mutex>
#include <sstream>

class CLetterModel;
class CTimerEventManager;
class CGameManager;
class CPlayer;
class CUIPlayerLetters;
class CBinaryBoolList;

struct TLetterAnimation
{
	TLetterAnimation(CLetterModel* letterModel, float distance, float height, size_t uiLetterIdx, int boardX, int boardY) : m_LetterModel(letterModel), m_Distance(distance), m_DestHeight(height), m_UILetterIdx(uiLetterIdx), m_BoardX(boardX), m_BoardY(boardY) {}

	enum ELetterAnimState { Waiting, InProgress, Finished };

	CLetterModel* m_LetterModel;
	float m_AminationTime = 0.f;
	float m_Distance;
	float m_DestHeight;
	int m_BoardX;
	int m_BoardY;
	size_t m_UILetterIdx;
	ELetterAnimState m_State = ELetterAnimState::Waiting;
};

struct TWordAnimation
{
	std::vector<TLetterAnimation> m_LetterAnimations;
	size_t m_CurrentLetterIdx;
	double m_LastAddedLetterTime = 0;
	std::string m_ID;
	CUIPlayerLetters* m_UIPlayerLetters;
	bool m_WaitForPrevLetter;

	static int m_CurrWordAnimID;

	TWordAnimation(CGameManager* gameManager, std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool waitForPrevLetter = true);
	int GetActiveLetterAnimCount();
};


class CWordAnimationManager
{
public:

	~CWordAnimationManager();

private:

	const float m_LetterAddInterval = 800.f; //TODO config
	const float m_LetterAnimTime = 350.f; //TODO config

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	double m_TimeSinceLastAddedLetter = 0;

	std::list<TWordAnimation> m_WordAnimations;
	std::list<size_t> m_UILetterIndices;

	std::mutex m_AnimListLock;

public:

	CWordAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager) : m_TimerEventManager(timerEventMgr), m_GameManager(gameManager) {}

	void SetLetterInProgress(TWordAnimation& word, TLetterAnimation& letter);
	bool HandleLetterAnimation(std::vector<TLetterAnimation>& letters, double timeFromPrevUpdate);
	bool AddWordAnimation(std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool waitForPrevLetter = true);
	void AnimateLettersEvent(double& timeFromStart, double& timeFromPrev);
	void AnimationFinished();
	void Reset();
	void SaveState(std::ofstream& fileStream);
	void LoadState(std::ifstream& fileStream);

	void ResetUsedLetterIndices() {m_UILetterIndices.clear();}
	void ResetUsedLetterIndex(size_t idx) {m_UILetterIndices.remove(idx);}
	bool Empty() {return m_WordAnimations.size() == 0;}
};
