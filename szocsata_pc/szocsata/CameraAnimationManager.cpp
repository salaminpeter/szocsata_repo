#include "stdafx.h"

#include "CameraAnimationManager.h"
#include "TimerEventManager.h"
#include "GameManager.h"
#include "Renderer.h"


CCameraAnimationManager::CCameraAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager) : m_GameManager(gameManager), m_TimerEventManager(timerEventMgr) 
{
}

void CCameraAnimationManager::AnimateCamera(double& timeFromStart, double& timeFromPrev)
{
	double Duration = 500; //TODO COnfig!
	float t = (timeFromStart / Duration);
	float d = std::sinf(t * glm::radians(90.f));

	float Tilt;
	float Rotation;
	float Zoom;
	float Move;

	float PrevTilt = m_CurrTilt;
	float PrevRotation = m_CurrRotation;
	float PrevZoom = m_CurrZoom;
	float PrevMove = m_CurrMoveDistance;

	Tilt = d * m_DestTilt - m_CurrTilt;
	Rotation = d * m_DestRotation - m_CurrRotation;
	Zoom = d * m_DestZoom - m_CurrZoom;
	Move = d * m_DestMoveDistance - m_CurrMoveDistance;

	bool EndAnimation = false;

	if (std::fabs(m_CurrTilt + Tilt) > std::fabs(m_DestTilt) || 
		std::fabs(m_CurrRotation + Rotation) > std::fabs(m_DestRotation) ||
		std::fabs(m_CurrZoom + Zoom) > std::fabs(m_DestZoom) ||
		std::fabs(m_CurrMoveDistance + Move) > std::fabs(m_DestMoveDistance) ||
		timeFromStart >= Duration)
	{
		Tilt = m_DestTilt - PrevTilt;
		Rotation = m_DestRotation - PrevRotation;
		Zoom = m_DestZoom - PrevZoom;
		Move = m_DestMoveDistance - PrevMove;
		EndAnimation = true;
	}
	
	m_CurrTilt += Tilt;
	m_CurrRotation += Rotation;
	m_CurrZoom += Zoom;
	m_CurrMoveDistance += Move;

	m_GameManager->GetRenderer()->CameraFitToViewAnim(Tilt, Rotation, Zoom, Move, m_MoveDirection);

	if (EndAnimation)
	{
		m_GameManager->GetRenderer()->ResetZoom();
		m_TimerEventManager->StopTimer("fit_to_view_animation");
		m_GameManager->GetRenderer()->CalculateScreenSpaceGrid();
	}
}

void CCameraAnimationManager::StartFitToScreenAnimation()
{
	m_CurrTilt = 0.f;
	m_CurrRotation = 0.f;
	m_CurrZoom = 0.f;
	m_CurrMoveDistance = 0.f;

	m_GameManager->GetRenderer()->GetFitToScreemProps(m_DestTilt, m_DestRotation, m_DestZoom, m_DestMoveDistance, m_MoveDirection);
	m_TimerEventManager->AddTimerEvent(this, &CCameraAnimationManager::AnimateCamera, nullptr, "fit_to_view_animation");
	m_TimerEventManager->StartTimer("fit_to_view_animation");
}

