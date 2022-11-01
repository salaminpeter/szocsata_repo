#pragma once

#include <glm/glm.hpp>

class CUIButton;
class CGameManager;
class CAnimationPath;
class CTimerEventManager;

class CScoreAnimationManager
{
public:
	
	CScoreAnimationManager(float textSize, CGameManager* gameManager, CTimerEventManager* timerEventManager);
	~CScoreAnimationManager();

	void SetScore(int score);
	void StartAnimation(float startX, float startY, int playerIdx, int score);
	void AnimateScore(double& timeFromStart, double& timeFromPrev);

	float GetTextSize() {return m_TextSize;}

	
private:

	const float m_FirstAnimTime = 3000;
	const float m_AnimTime = 700;

	CAnimationPath* m_AnimationPath;
	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	int m_PlayerIdx;
	int m_PassedTime = 0;
	float m_TextSize;
	float m_CurrentSize;
	bool m_FirstAnimPhase = false;
	float m_FirstAnimLength;
	glm::vec2 m_StartPosition;
};