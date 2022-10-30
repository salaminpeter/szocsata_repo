#include "stdafx.h"
#include "ButtonAnimationManager.h"
#include "TimerEventManager.h"
#include "UIElement.h"

CButtonAnimationManager::CButtonAnimationManager(CTimerEventManager* timerEventManager) : m_TimerEventManager(timerEventManager) 
{
	m_TimerEventManager->AddTimerEvent(this, &CButtonAnimationManager::HandleAnimation, nullptr, "button_animation");
	m_TimerEventManager->PauseTimer("button_animation");
}

void CButtonAnimationManager::HandleAnimation(double& timeFromStart, double& timeFromPrev)
{
	m_ButtonAnimation.m_TimeSinceAnimStart += timeFromPrev;

	if (!m_ButtonAnimation.m_AnimationFinished)
	{
		bool AnimFinished = m_ButtonAnimation.m_TimeSinceAnimStart > m_AnimTime;
		float NewWidth;
		float NewHeight;

		if (!AnimFinished)
		{ 
			float mul = std::sin(glm::radians((static_cast<double>(m_ButtonAnimation.m_TimeSinceAnimStart) / m_AnimTime) * 90.0));
			NewWidth = m_ButtonAnimation.m_StartWidth + mul * (m_ButtonAnimation.m_DestWidth - m_ButtonAnimation.m_StartWidth);
			NewHeight = m_ButtonAnimation.m_StartHeight + mul* (m_ButtonAnimation.m_DestHeight - m_ButtonAnimation.m_StartHeight);
		}
		else
		{
			m_TimerEventManager->PauseTimer("button_animation");

			NewWidth = m_ButtonAnimation.m_DestWidth;
			NewHeight = m_ButtonAnimation.m_DestHeight;
			m_ButtonAnimation.m_AnimationFinished = true;

			bool IsReversdeAnimation = m_ButtonAnimation.m_StartWidth > m_ButtonAnimation.m_DestWidth;

			if (IsReversdeAnimation)
			{
				m_AnimationInProgress = false;
				RunAnimationFinishEvent(m_ButtonAnimation.m_Button);
				return;
			}
		}
		
		m_ButtonAnimation.m_Button->SetSizeWithChildren(NewWidth, NewHeight);
	}

	if (m_ButtonAnimation.m_AnimationFinished && m_ButtonAnimation.m_RunReverseAnimation)
	{
		m_ButtonAnimation.m_TimeSinceAnimStart = 0;
		float StartWidth = m_ButtonAnimation.m_StartWidth;
		float StartHeight = m_ButtonAnimation.m_StartHeight;
		m_ButtonAnimation.m_StartWidth = m_ButtonAnimation.m_DestWidth;
		m_ButtonAnimation.m_StartHeight = m_ButtonAnimation.m_DestHeight;
		m_ButtonAnimation.m_DestWidth = StartWidth;
		m_ButtonAnimation.m_DestHeight = StartHeight;
		m_ButtonAnimation.m_AnimationFinished = false;
		m_ButtonAnimation.m_RunReverseAnimation = false;
		m_TimerEventManager->StartTimer("button_animation");
	}
}

void CButtonAnimationManager::AddButtonAnimation(CUIElement* button)
{
	if (!m_AnimationInProgress)
	{
		m_AnimationInProgress = true;
		m_ButtonAnimation = TButtonAnimation(button);
		m_TimerEventManager->StartTimer("button_animation");
	}
}

CButtonAnimationManager::TButtonAnimation::TButtonAnimation(CUIElement* button) : m_Button(button)
{
	m_StartWidth = button->GetWidth();
	m_DestWidth = button->GetWidth() * m_MaxScale;
	m_StartHeight = button->GetHeight();
	m_DestHeight = button->GetHeight() * m_MaxScale;
}

CButtonAnimationManager::TButtonAnimation& CButtonAnimationManager::TButtonAnimation::operator=(const TButtonAnimation& rhs)
{
	m_StartWidth = rhs.m_StartWidth;
	m_StartHeight = rhs.m_StartHeight;
	m_DestWidth = rhs.m_DestWidth;
	m_DestHeight = rhs.m_DestHeight;
	m_Button = rhs.m_Button;
	m_AnimationFinished = rhs.m_AnimationFinished;
	m_TimeSinceAnimStart = rhs.m_TimeSinceAnimStart; 

	return *this;
}

void CButtonAnimationManager::RunAnimationFinishEvent(CUIElement* button)
{
	auto Elem = m_AnimFinishEvents.find(button);

	if (Elem == m_AnimFinishEvents.end())
		return;

	Elem->second->HandleEvent();
}

void CButtonAnimationManager::EnableReverseAnimation(CUIElement* button)
{
	m_ButtonAnimation.m_RunReverseAnimation = true;
	m_TimerEventManager->StartTimer("button_animation");
}


