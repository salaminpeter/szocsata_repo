#include "stdafx.h"
#include "PlayerLetterAnimationManager.h"
#include "TimerEventManager.h"
#include "UIElement.h"


void CPlayerLetterAnimationManager::StartAnimations()
{
	if (m_PlayerLetterAnimations.size())
	{
		m_CurrentLetterIdx = 0;
		m_TimerEventManager->AddTimerEvent(this, &CPlayerLetterAnimationManager::AnimatePlayerLetter, nullptr, "player_letter_animation");
		m_TimerEventManager->StartTimer("player_letter_animation");
	}
}

void CPlayerLetterAnimationManager::AnimatePlayerLetter(double& timeFromStart, double& timeFromPrev)
{
	const double AnimTime = 300.f; //TODO config

	float Scale = std::sinf((timeFromStart / AnimTime) * (90.f * 3.14f / 180.f)) * m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale;
	m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->Scale(timeFromStart < AnimTime ? Scale : m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale);

	if (timeFromStart > AnimTime)
	{
		m_TimerEventManager->StopTimer("player_letter_animation");

		if (m_CurrentLetterIdx + 1 == m_PlayerLetterAnimations.size())
		{
			m_PlayerLetterAnimations.clear();
			return;
		}

		m_CurrentLetterIdx++;
		m_TimerEventManager->StartTimer("player_letter_animation");
	}
}

void CPlayerLetterAnimationManager::AddAnimation(CUIElement* playerLEtter, float destScale)
{
	m_PlayerLetterAnimations.emplace_back(playerLEtter, destScale);
}
