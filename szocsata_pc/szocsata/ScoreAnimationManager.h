#pragma once

#include <glm/glm.hpp>
#include <sstream>
class CUIButton;
class CGameManager;
class CAnimationPath;
class CTimerEventManager;

class CScoreAnimationManager
{
public:
	
	CScoreAnimationManager(float size, CGameManager* gameManager, CTimerEventManager* timerEventManager);
	~CScoreAnimationManager();

	void SetScore(int score);
	void StartAnimation();
	void SetProperties(float startX, float startY, int playerIdx, int score, int passedTime, bool firstAnimPhase, int currSize = -1);
	void SetProperties();
	void AnimateScore(double& timeFromStart, double& timeFromPrev);
	void SaveState(std::ofstream& fileStream);
	void LoadState(std::ifstream& fileStream);

	float GetSize() {return m_Size;}
	float HasAnimation() {return m_AnimationInProgress;}

	
private:

	const float m_FirstAnimTime = 3000;
	const float m_AnimTime = 700;

	CAnimationPath* m_AnimationPath;
	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	int m_PlayerIdx;
	int m_PassedTime = 0;
	float m_Size;
	float m_CurrentSize;
	bool m_FirstAnimPhase = false;
	float m_FirstAnimLength;
	glm::vec2 m_StartPosition;
	bool m_AnimationInProgress = false;
	int m_Score;
};