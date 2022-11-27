#pragma once

#include <list>
#include <atomic>
#include <map>

#include "Event.h"

class CUIElement;
class CTimerEventManager;

class CButtonAnimationManager
{
public:
	
	CButtonAnimationManager(CTimerEventManager* timerEventManager);

	struct TButtonAnimation
	{
		CUIElement* m_Button;
		int m_TimeSinceAnimStart = 0;
		float m_StartWidth;
		float m_DestWidth;
		float m_StartHeight;
		float m_DestHeight;
		bool m_RunReverseAnimation = false;
		bool m_AnimationFinished = false;

		const float m_MaxScale = 0.7;

		TButtonAnimation() = default;
		TButtonAnimation(CUIElement* button);
		TButtonAnimation& operator=(const TButtonAnimation& rhs);
	};

	void AddButtonAnimation(CUIElement* button);
	void HandleAnimation(double& timeFromStart, double& timeFromPrev);
	void EnableReverseAnimation(CUIElement* button);
	void RunAnimationFinishEvent(CUIElement* button);

	template <typename ClassType, typename... ArgTypes>
	void SetEvent(CUIElement* button, ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, ArgTypes&&... args)
	{
		if (m_AnimFinishEvents.find(button) != m_AnimFinishEvents.end())
			return;

		m_AnimFinishEvents[button] = new CEvent<ClassType, ArgTypes...>(funcClass, funcPtr, std::forward<ArgTypes>(args)...);
	}

	
private:
	
	const int m_AnimTime = 150;
	std::atomic<bool> m_AnimationInProgress = false;
	std::map<CUIElement*, CEventBase*> m_AnimFinishEvents;

	TButtonAnimation m_ButtonAnimation;
	CTimerEventManager* m_TimerEventManager;
};
