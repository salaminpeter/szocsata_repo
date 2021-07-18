#include "stdafx.h"
#include "PlayerLetterAnimationManager.h"
#include "TimerEventManager.h"
#include "GameManager.h"
#include "UIManager.h"
#include "UIElement.h"


void CPlayerLetterAnimationManager::StartAnimations()
{
	if (m_PlayerLetterAnimations.size())
	{
		m_CurrentLetterIdx = 0;
		m_TimerEventManager->AddTimerEvent(this, &CPlayerLetterAnimationManager::AnimatePlayerLetter, &CPlayerLetterAnimationManager::AnimFinishedEvent, "player_letter_animation");
		m_TimerEventManager->StartTimer("player_letter_animation");
	}
}


void CPlayerLetterAnimationManager::AnimFinishedEvent()
{
	if (m_GameManager->PlayerLetterAnimationFinished())
		m_GameManager->ShowNextPlayerPopup();
}


void CPlayerLetterAnimationManager::AnimatePlayerLetter(double& timeFromStart, double& timeFromPrev)
{
	const double AnimTime = 800.f; //TODO config

	float Mul = std::sinf((timeFromStart / AnimTime) * (90.f * 3.14f / 180.f));
	float Scale = Mul * m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale;
	float XPos = Mul * (m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestX - m_PlayerLetterAnimations[m_CurrentLetterIdx].m_StartX) + m_PlayerLetterAnimations[m_CurrentLetterIdx].m_StartX;
	float YPos = Mul * (m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestY - m_PlayerLetterAnimations[m_CurrentLetterIdx].m_StartY) + m_PlayerLetterAnimations[m_CurrentLetterIdx].m_StartY;
	m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->Scale(timeFromStart < AnimTime ? Scale : m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale);
	m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->SetPosAndSize(XPos, YPos, Scale, Scale);

	if (timeFromStart > AnimTime)
	{
		m_TimerEventManager->StopTimer("player_letter_animation");

		m_GameManager->GetUIManager()->SetTileCounterValue(m_GameManager->GetUIManager()->GetTileCounterValue() - 1);

		if (m_CurrentLetterIdx + 1 == m_PlayerLetterAnimations.size())
		{
			m_PlayerLetterAnimations.clear();
			return;
		}

		m_CurrentLetterIdx++;
		m_TimerEventManager->StartTimer("player_letter_animation");
	}
}

void CPlayerLetterAnimationManager::AddAnimation(CUIElement* playerLEtter, float destScale, float startX, float startY, float destX, float destY)
{
	m_PlayerLetterAnimations.emplace_back(playerLEtter, destScale, startX, startY, destX, destY);
}
