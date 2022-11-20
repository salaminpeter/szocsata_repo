#include "stdafx.h"
#include "DimmBGAnimationManager.h"
#include "GameManager.h"
#include "TimerEventManager.h"

#include <cmath>

CDimmBGAnimationManager::CDimmBGAnimationManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : m_TimerEventManager(timerEventManager), m_GameManager(gameManager)
{
	m_TimerEventManager->AddTimerEvent(this, &CDimmBGAnimationManager::AnimateBackground, &CDimmBGAnimationManager::AnimFinishedEvent, "dimming_animation");
}


void CDimmBGAnimationManager::AnimateBackground(double& timeFromStart, double& timeFromPrev)
{
	const double AnimateDuration = 400; //TODO config

	if (timeFromStart >= AnimateDuration)
	{
		m_TimerEventManager->FinishTimer("dimming_animation");
		return;
	}

	float Opacity = std::sinf((timeFromStart / AnimateDuration) * (90.f * (3.14f / 180.f))) * .9f;

	if (!m_FadeIn)
		Opacity = .9f - Opacity;

	m_GameManager->SetDimmPanelOpacity(Opacity);
}

void CDimmBGAnimationManager::StartAnimation(bool fadiIn)
{
	m_FadeIn = fadiIn;
	m_TimerEventManager->StartTimer("dimming_animation");
}

void CDimmBGAnimationManager::AnimFinishedEvent()
{
	m_GameManager->SetDimmPanelOpacity(m_FadeIn ? .9f : 0.f);
}
