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
	double Duration = 1000; //TODO 15000 COnfig!
	float d = std::sinf((timeFromStart / Duration) * glm::radians(90.f)); 
	float Tilt;
	float Rotation;
	float Zoom;

	bool EndAnimation = false;
	float LookAtYAxisAngle = m_GameManager->GetRenderer()->GetLookAtYAxisAngle();
	float CamTiltAngle = m_GameManager->GetRenderer()->GetCameraTiltAngle();

	//rotate / tilt
	if (Duration - timeFromStart < 0.1)
	{
		EndAnimation = true;
		Tilt = 90.f - CamTiltAngle;
		Rotation = LookAtYAxisAngle;
	}
	else
	{
		Rotation = (LookAtYAxisAngle * timeFromPrev) / (Duration - timeFromStart);
		Tilt = ((90.f - CamTiltAngle) * timeFromPrev) / (Duration - timeFromStart);
	}

	//zoom
	m_GameManager->GetRenderer()->RotateCamera(Rotation, Tilt);
	float ZoomDist = m_GameManager->GetRenderer()->GetFitToViewZoomDistance();

	if (Duration - timeFromStart < 0.1)
		Zoom = ZoomDist;
	else
		Zoom = (ZoomDist * timeFromPrev) / (Duration - timeFromStart);
	
	m_GameManager->GetRenderer()->ZoomCamera(Zoom, 0.f, 0.f, true, true);

	if (EndAnimation)
	{
		m_GameManager->GetRenderer()->ResetZoom();
		m_TimerEventManager->StopTimer("fit_to_view_animation");
	}
}

void CCameraAnimationManager::StartFitToScreenAnimation()
{
	m_CurrTilt = 0.f;
	m_CurrRotation = 0.f;
	m_CurrZoom = 0.f;

	m_GameManager->GetRenderer()->GetFitToScreemProps(m_DestTilt, m_DestRotation, m_DestZoom);
	m_TimerEventManager->AddTimerEvent(this, &CCameraAnimationManager::AnimateCamera, nullptr, "fit_to_view_animation");
	m_TimerEventManager->StartTimer("fit_to_view_animation");
}

