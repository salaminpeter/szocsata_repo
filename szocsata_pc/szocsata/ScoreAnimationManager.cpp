#include "stdafx.h"
#include "ScoreAnimationManager.h"
#include "UIText.h"
#include "UIButton.h"
#include "UIManager.h"
#include "GameManager.h"
#include "AnimationPath.h"
#include "TimerEventManager.h"
#include <sstream>

CScoreAnimationManager::CScoreAnimationManager(float size, CGameManager* gameManager, CTimerEventManager* timerEventManager) :
	m_TimerEventManager(timerEventManager),
	m_GameManager(gameManager),
	m_Size(size)
{
	m_AnimationPath = new CAnimationPath();
	m_FirstAnimLength = m_GameManager->m_SurfaceHeigh / 5;
	m_TimerEventManager->AddTimerEvent(this, &CScoreAnimationManager::AnimateScore, nullptr, "score_animation_timer");
}

CScoreAnimationManager::~CScoreAnimationManager()
{
	delete m_AnimationPath;
}

void CScoreAnimationManager::SetProperties()
{
	SetProperties(m_StartPosition.x, m_StartPosition.y, m_PlayerIdx, m_Score, m_PassedTime, m_FirstAnimPhase, m_CurrentSize);
}

void CScoreAnimationManager::SetProperties(float startX, float startY, int playerIdx, int score, int passedTime, bool firstAnimPhase, int currSize)
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());

	CUIButton* ScoreButton = static_cast<CUIButton*>(m_GameManager->GetUIManager()->GetUIElement(L"ui_score_btn"));
	m_Size = ScoreButton->GetWidth();
	m_CurrentSize = currSize  < 0 ? m_Size : currSize;

	if (startX < m_Size / 2.f)
		startX = m_Size * 2;

	if (startY > m_GameManager->m_SurfaceHeigh - m_FirstAnimLength - m_Size * 2.f)
		startY = m_GameManager->m_SurfaceHeigh - m_FirstAnimLength - m_Size * 2.f;

	if (startX > m_GameManager->m_SurfaceWidth / 2 - m_Size / 2)
		startX = m_GameManager->m_SurfaceWidth / 2 - m_Size * 2;

	if (startY < m_Size / 2.f)
		startY = m_Size * 2;

	m_Score = score;
	m_PassedTime = passedTime;
	m_FirstAnimPhase = firstAnimPhase;

	std::vector<glm::vec2> AnimPoints;

	//p0
	AnimPoints.emplace_back(startX, startY);

	//p1
	AnimPoints.emplace_back(startX, startY + m_FirstAnimLength);

	std::wstringstream ss;
	ss << L"ui_player_score_icon_" << playerIdx;
	CUIElement* ScorePanel = m_GameManager->GetUIManager()->GetUIElement(L"ui_score_panel");
	glm::vec2 ScorePanelPos = ScorePanel->GetAbsolutePosition();
	CUIElement* PlayerScoreIcon = m_GameManager->GetUIManager()->GetUIElement(ss.str().c_str());
	glm::vec2 IconPos = PlayerScoreIcon->GetAbsolutePosition();

	float Distance = glm::distance(AnimPoints.back(), IconPos);
	float p3x;
	float p3y;
	glm::vec2 NewPoint;
	glm::vec2 LastPoint;

	if (startY + m_FirstAnimLength < IconPos.y)
	{
		p3x = ScorePanelPos.x - ScorePanel->GetWidth() / 2;
		p3y = ScorePanelPos.y + ScorePanel->GetHeight() * 2.f;
		NewPoint = glm::vec2((p3x + startX) / 2.f, m_GameManager->m_SurfaceHeigh - m_Size * 2.f);
		LastPoint = glm::vec2(IconPos.x - 20.f, 20.f);
	}
	else
	{
		p3x = ScorePanelPos.x - ScorePanel->GetWidth() / 2;
		p3y = ScorePanelPos.y - ScorePanel->GetHeight() * 2.f;
		NewPoint = glm::vec2((p3x + startX) / 2.f, m_Size * 2.f);
		LastPoint = glm::vec2(IconPos.x + 20.f, -20.f);
	}

	//p2
	AnimPoints.emplace_back(NewPoint.x, NewPoint.y);

	//p3
	AnimPoints.emplace_back(p3x, p3y);

	//p4
	if (startY + m_FirstAnimLength < IconPos.y)
		AnimPoints.emplace_back(p3x + 2 * std::fabs(p3y - IconPos.y) / 4.f, (p3y + IconPos.y) / 2.f);
	else
		AnimPoints.emplace_back(p3x + std::fabs(p3y - IconPos.y) / 4.f, (p3y + IconPos.y) / 2.f);

	//p5
	AnimPoints.emplace_back(IconPos.x, IconPos.y);

	//p6
	AnimPoints.emplace_back(LastPoint.x, LastPoint.y);

	m_AnimationPath->CreatePath(AnimPoints, 2);

	ScoreButton->SetPosAndSize(startX, startY, m_CurrentSize, m_CurrentSize);
	SetScore(score);

	m_StartPosition = glm::vec2(startX, startY);
}

void CScoreAnimationManager::StartAnimation()
{
	m_AnimationInProgress = true;
	CUIButton *ScoreButton = static_cast<CUIButton *>(m_GameManager->GetUIManager()->GetUIElement(L"ui_score_btn"));
	ScoreButton->SetVisible(true);
	m_TimerEventManager->StartTimer("score_animation_timer");
}


void CScoreAnimationManager::SaveState(std::ofstream& fileStream)
{
	m_TimerEventManager->PauseTimer("score_animation_timer");

	fileStream.write((char *)&m_AnimationInProgress, sizeof(bool));

	if (!m_AnimationInProgress)
		return;

	fileStream.write((char *)&m_PlayerIdx, sizeof(int));
	fileStream.write((char *)&m_PassedTime, sizeof(int));
	fileStream.write((char *)&m_Size, sizeof(float));
	fileStream.write((char *)&m_CurrentSize, sizeof(float));
	fileStream.write((char *)&m_FirstAnimPhase, sizeof(bool));
	fileStream.write((char *)&m_FirstAnimLength, sizeof(float));
	fileStream.write((char *)&m_StartPosition.x, sizeof(float));
	fileStream.write((char *)&m_StartPosition.y, sizeof(float));
	fileStream.write((char *)&m_Score, sizeof(int));
}

void CScoreAnimationManager::LoadState(std::ifstream& fileStream)
{
	m_TimerEventManager->PauseTimer("score_animation_timer");

	fileStream.read((char *)&m_AnimationInProgress, sizeof(bool));

	if (!m_AnimationInProgress)
		return;

	fileStream.read((char *)&m_PlayerIdx, sizeof(int));
	fileStream.read((char *)&m_PassedTime, sizeof(int));
	fileStream.read((char *)&m_Size, sizeof(float));
	fileStream.read((char *)&m_CurrentSize, sizeof(float));
	fileStream.read((char *)&m_FirstAnimPhase, sizeof(bool));
	fileStream.read((char *)&m_FirstAnimLength, sizeof(float));
	fileStream.read((char *)&m_StartPosition.x, sizeof(float));
	fileStream.read((char *)&m_StartPosition.y, sizeof(float));
	fileStream.read((char *)&m_Score, sizeof(int));
}

void CScoreAnimationManager::AnimateScore(double& timeFromStart, double& timeFromPrev)
{
    const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());
    {
        CUIButton *ScoreButton = static_cast<CUIButton *>(m_GameManager->GetUIManager()->GetUIElement(L"ui_score_btn"));

        m_PassedTime += timeFromPrev;
        bool AnimEnded = m_PassedTime >= m_AnimTime;

        if (m_FirstAnimPhase)
        {
            float Mul = sinf((m_PassedTime / m_FirstAnimTime) * glm::radians(90.f));
            glm::vec2 AnimPos = m_StartPosition + glm::vec2(0.f, m_FirstAnimLength * Mul);
			float NewSize = m_Size + m_Size * (Mul * .8f);
			ScoreButton->SetPosition(AnimPos.x, AnimPos.y);
			ScoreButton->SetSizeWithChildren(NewSize, NewSize);
			AnimEnded = m_PassedTime >= m_FirstAnimTime;
        }
        else
        {
            float Mul = 1.f - sinf(glm::radians(90.f) + (m_PassedTime / m_AnimTime) * glm::radians(90.f));
			float NewSize = m_CurrentSize - m_CurrentSize * Mul *.8f;
            glm::vec2 AnimPos = m_AnimationPath->GetPathPoint(Mul);
			ScoreButton->SetPosition(AnimPos.x, AnimPos.y);
			ScoreButton->SetSizeWithChildren(NewSize, NewSize);
			AnimEnded = m_PassedTime >= m_AnimTime;
        }

        if (AnimEnded)
        {
            if (m_FirstAnimPhase)
            {
                m_FirstAnimPhase = false;
                m_PassedTime = 0;
                m_CurrentSize = ScoreButton->GetWidth();
                return;
            }
            else
            {
                m_AnimationInProgress = false;
                m_GameManager->UpdatePlayerScores();
                m_TimerEventManager->PauseTimer("score_animation_timer");
				ScoreButton->SetVisible(false);
				ScoreButton->SetSizeWithChildren(m_Size, m_Size);
				m_GameManager->SetTaskFinished("score_animation_task");
				return;
            }
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

