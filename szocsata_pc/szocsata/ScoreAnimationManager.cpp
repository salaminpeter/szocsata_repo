#include "stdafx.h"
#include "ScoreAnimationManager.h"
#include "UIText.h"
#include "UIButton.h"
#include "UIManager.h"
#include "GameManager.h"
#include "AnimationPath.h"
#include "TimerEventManager.h"
#include <sstream>

CScoreAnimationManager::CScoreAnimationManager(float textSize, CGameManager* gameManager, CTimerEventManager* timerEventManager) :
	m_TimerEventManager(timerEventManager),
	m_GameManager(gameManager),
	m_TextSize(textSize)
{
	m_AnimationPath = new CAnimationPath();
	m_FirstAnimLength = m_GameManager->m_SurfaceHeigh / 5;
}

CScoreAnimationManager::~CScoreAnimationManager()
{
	delete m_AnimationPath;
}

void CScoreAnimationManager::StartAnimation(float startX, float startY, int playerIdx, int score)
{
	std::vector<glm::vec2> Points;
	Points.reserve(7);

	if (startX < m_TextSize / 2.f)
		startX = m_TextSize * 2;

	if (startY > m_GameManager->m_SurfaceHeigh - m_FirstAnimLength - 150.f)
		startY = m_GameManager->m_SurfaceHeigh - m_FirstAnimLength - 150.f;

	if (startY < m_TextSize / 2.f)
		startY = m_TextSize * 2;

	//p0
	Points.emplace_back(startX, startY + 20.f);

	//p1
	Points.emplace_back(startX, startY + m_FirstAnimLength);

	std::wstringstream ss;
	ss << L"ui_player_score_icon_" << playerIdx;
	CUIElement* ScorePanel = m_GameManager->GetUIManager()->GetUIElement(L"ui_score_panel");
	glm::vec2 ScorePanelPos = ScorePanel->GetAbsolutePosition();
	CUIElement* PlayerScoreIcon = m_GameManager->GetUIManager()->GetUIElement(ss.str().c_str());
	glm::vec2 IconPos = PlayerScoreIcon->GetAbsolutePosition();

	float Distance = glm::distance(Points.back(), IconPos);
	float p3x = ScorePanelPos.x - ScorePanel->GetWidth() / 2;
	float p3y = ScorePanelPos.y + ScorePanel->GetHeight() * 2.f;
	glm::vec2 NewPoint = glm::vec2((p3x + startX) / 2.f, m_GameManager->m_SurfaceHeigh - 100);

	//p2
	Points.emplace_back(NewPoint.x, NewPoint.y);

	//p3
	Points.emplace_back(p3x, p3y);

	//p4
	Points.emplace_back(p3x + (p3y - IconPos.y) / 4.f, (p3y + IconPos.y) / 2.f);

	//p5
	Points.emplace_back(IconPos.x, IconPos.y);

	//p6
	Points.emplace_back(IconPos.x - 20, IconPos.y - 20);

	m_AnimationPath->CreatePath(Points, 2);

	CUIButton* ScoreButton = static_cast<CUIButton*>(m_GameManager->GetUIManager()->GetUIElement(L"ui_score_btn"));
	ScoreButton->SetPosAndSize(startX, startY, ScoreButton->GetWidth(), ScoreButton->GetHeight());
	ScoreButton->SetVisible(true);
	SetScore(score);

	m_StartPosition = glm::vec2(startX, startY);
	m_PassedTime = 0;
	m_FirstAnimPhase = true;
	m_TimerEventManager->AddTimerEvent(this, &CScoreAnimationManager::AnimateScore, nullptr, "score_animation_timer");
	m_TimerEventManager->StartTimer("score_animation_timer");
}


void CScoreAnimationManager::AnimateScore(double& timeFromStart, double& timeFromPrev)
{
	CUIButton* ScoreButton = static_cast<CUIButton*>(m_GameManager->GetUIManager()->GetUIElement(L"ui_score_btn"));

	m_PassedTime += timeFromPrev;
	bool AnimEnded = m_PassedTime >= m_AnimTime;

	if (m_FirstAnimPhase)
	{
		float Mul = sinf((m_PassedTime / m_FirstAnimTime) * glm::radians(90.f));
		glm::vec2 AnimPos = m_StartPosition + glm::vec2(0.f, m_FirstAnimLength * Mul);
		ScoreButton->SetPosAndSize(AnimPos.x, AnimPos.y, m_TextSize + m_TextSize * (Mul * .5f), m_TextSize + m_TextSize * (Mul * .5f));
		AnimEnded = m_PassedTime >= m_FirstAnimTime;
	}
	else
	{
		float Mul = 1.f - sinf(glm::radians(90.f) + (m_PassedTime / m_AnimTime) * glm::radians(90.f));
		glm::vec2 AnimPos = m_AnimationPath->GetPathPoint(Mul);
		ScoreButton->SetPosAndSize(AnimPos.x, AnimPos.y, m_CurrentSize - m_CurrentSize * Mul * .6f, m_CurrentSize - m_CurrentSize * Mul * .6f);
		AnimEnded = m_PassedTime >= m_AnimTime;
	}

	if (AnimEnded)
	{
		if (m_FirstAnimPhase)
		{
			m_FirstAnimPhase = false;
			m_PassedTime = 0;
			m_CurrentSize = ScoreButton->GetWidth();
		}
		else
		{
			ScoreButton->SetVisible(false);
			m_GameManager->UpdatePlayerScores();
			m_TimerEventManager->StopTimer("score_animation_timer");
			m_GameManager->SetTaskFinished("score_animation_task");
		}
	}
}

void CScoreAnimationManager::SetScore(int score)
{
	CUIButton* ScoreButton = static_cast<CUIButton*>(m_GameManager->GetUIManager()->GetUIElement(L"ui_score_btn"));
	
	std::wstringstream ss;
	ss << score;
	ScoreButton->SetText(ss.str().c_str());
}

