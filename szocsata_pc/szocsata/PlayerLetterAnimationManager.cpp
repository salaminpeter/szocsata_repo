#include "stdafx.h"
#include "PlayerLetterAnimationManager.h"
#include "TimerEventManager.h"
#include "GameManager.h"
#include "UIManager.h"
#include "UIElement.h"

#include <iostream>
#include <fstream>


void CPlayerLetterAnimationManager::StartAnimations()
{
	if (m_PlayerLetterAnimations.size())
	{
		m_CurrentLetterIdx = 0;
		m_PrevLetterIdx = 1;
		m_TimerEventManager->AddTimerEvent(this, &CPlayerLetterAnimationManager::AnimatePlayerLetter, &CPlayerLetterAnimationManager::AnimFinishedEvent, "player_letter_animation");
		m_TimerEventManager->StartTimer("player_letter_animation");
	}
}

void CPlayerLetterAnimationManager::AnimFinishedEvent()
{
	m_GameManager->SetTaskFinished("finish_player_deal_letters_task");
}


void CPlayerLetterAnimationManager::AnimatePlayerLetter(double& timeFromStart, double& timeFromPrev)
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());

	const double AnimTime = 1100.f; //TODO config

	if (m_TimeSinceAnimStart == 0)
		m_TimeSinceAnimStart = timeFromStart;
	else
		m_TimeSinceAnimStart += timeFromPrev;

	float Mul = std::sinf((m_TimeSinceAnimStart / AnimTime) * (90.f * 3.14f / 180.f));
	float Scale = Mul * m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale;
	glm::vec2 CurrPos = m_PlayerLetterAnimations[m_CurrentLetterIdx].m_AnimationPath.GetPathPoint(Mul);
	float XPos = CurrPos.x;
	float YPos = CurrPos.y;
	m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->Scale(m_TimeSinceAnimStart < AnimTime ? Scale : m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale);
	m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->SetPosAndSize(XPos, YPos, Scale, Scale);

	if (m_TimeSinceAnimStart >= AnimTime)
		m_PlayerLetterAnimations[m_CurrentLetterIdx].m_Finished = true;

	if (m_CurrentLetterIdx != m_PrevLetterIdx)
	{
		m_PrevLetterIdx = m_CurrentLetterIdx;

		if (m_GameManager->GetUIManager()->GetTileCounterValue() >= 1)
			m_GameManager->GetUIManager()->SetTileCounterValue(m_GameManager->GetUIManager()->GetTileCounterValue() - 1);
	}

	if (m_TimeSinceAnimStart > AnimTime)
	{
		//set final position
		m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->Scale(m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestScale);
		m_PlayerLetterAnimations[m_CurrentLetterIdx].m_PlayerLetter->SetPosAndSize(m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestX, m_PlayerLetterAnimations[m_CurrentLetterIdx].m_DestY, Scale, Scale);

		m_TimerEventManager->StopTimer("player_letter_animation");
		m_TimeSinceAnimStart = 0;

		if (m_CurrentLetterIdx + 1 == m_PlayerLetterAnimations.size())
		{
			if (m_GameManager->GetLetterPoolCount() == 0)
				m_GameManager->GetUIManager()->SetTileCounterValue(0);

			m_PlayerLetterAnimations.clear();
			return;
		}

		m_CurrentLetterIdx++;
		m_TimerEventManager->StartTimer("player_letter_animation");
	}
}

void CPlayerLetterAnimationManager::AddAnimation(CUIElement* playerLEtter, float destScale, float startX, float startY, float destX, float destY)
{
	m_PlayerLetterAnimations.emplace_back(playerLEtter, m_GameManager, destScale, startX, startY, destX, destY);
	m_PlayerLetterAnimations.back().m_PlayerLetter->SetPosAndSize(m_PlayerLetterAnimations.back().m_StartX, m_PlayerLetterAnimations.back().m_StartY, 0.f, 0.f);
}

bool CPlayerLetterAnimationManager::Empty()
{
	return (m_PlayerLetterAnimations.size() == 0);
}

void CPlayerLetterAnimationManager::SaveState(std::ofstream& fileStream)
{
	size_t LeterAnimCount = 0;

	for (auto LetterAnim : m_PlayerLetterAnimations)
		LeterAnimCount += LetterAnim.m_Finished ? 0 : 1;

	m_TimerEventManager->PauseTimer("player_letter_animation");
	fileStream.write((char *)&LeterAnimCount, sizeof(size_t));

	if (LeterAnimCount == 0)
		return;

	fileStream.write((char *)&m_TimeSinceAnimStart, sizeof(int));

	for (auto LetterAnim : m_PlayerLetterAnimations)
	{
		if (LetterAnim.m_Finished)
			continue;

		std::wstring LetterId = LetterAnim.m_PlayerLetter->GetID();
		int IdLength = LetterId.length();

		fileStream.write((char *)&LetterAnim.m_DestScale, sizeof(float));
		fileStream.write((char *)&LetterAnim.m_StartX, sizeof(float));
		fileStream.write((char *)&LetterAnim.m_StartY, sizeof(float));
		fileStream.write((char *)&LetterAnim.m_DestX, sizeof(float));
		fileStream.write((char *)&LetterAnim.m_DestY, sizeof(float));
		fileStream.write((char *)&IdLength, sizeof(int));

		for (int i = 0; i < IdLength; ++i)
			fileStream.write((char *)&LetterId[i], sizeof(wchar_t));
	}
}

void CPlayerLetterAnimationManager::LoadState(std::ifstream& fileStream)
{
#ifndef PLATFORM_ANDROID
	#define size_t int64_t
	#define wchar_t char32_t
#endif
	m_TimerEventManager->PauseTimer("player_letter_animation");

	size_t LetterAnimCount;
	fileStream.read((char *)&LetterAnimCount, sizeof(size_t));

	if (LetterAnimCount == 0)
		return;

	fileStream.read((char *)&m_TimeSinceAnimStart, sizeof(int));

	for (int i = 0; i < LetterAnimCount; ++i)
	{
		float DestScale;
		float StartX;
		float StartY;
		float DestX;
		float DestY;
		int IdLength;

		fileStream.read((char *)&DestScale, sizeof(float));
		fileStream.read((char *)&StartX, sizeof(float));
		fileStream.read((char *)&StartY, sizeof(float));
		fileStream.read((char *)&DestX, sizeof(float));
		fileStream.read((char *)&DestY, sizeof(float));
		fileStream.read((char *)&IdLength, sizeof(int));

		std::wstring LetterId;
		wchar_t IdChar;

		for (int i = 0; i < IdLength; ++i)
		{
			fileStream.read((char *)&IdChar, sizeof(wchar_t));
			LetterId += IdChar;
		}

		CUIElement* LetterElement = m_GameManager->GetUIManager()->GetUIElement(LetterId.c_str());
		AddAnimation(LetterElement, DestScale, StartX, StartY, DestX, DestY);
		m_CurrentLetterIdx = 0;
		m_PrevLetterIdx = 1;
	}

#ifndef PLATFORM_ANDROID
	#undef size_t
	#undef wchar_t
#endif
}

CPlayerLetterAnimationManager::TPlayerLetterAnimation::TPlayerLetterAnimation(CUIElement *playerLetter, CGameManager *gameManager, float destScale, float startX,  float startY, float destX, float destY) :
		m_PlayerLetter(playerLetter),
		m_DestScale(destScale),
		m_StartX(startX),
		m_StartY(startY),
		m_DestX(destX),
		m_DestY(destY),
		m_Finished(false)
{
	bool DestinationLeft = destX < startX;
	std::vector<glm::vec2> AnimPathPoints;
	AnimPathPoints.reserve(7);
	glm::vec2 TileCounterSize = gameManager->GetUIManager()->GetTileCounterSize() / 2.f;

	//p0
	AnimPathPoints.emplace_back(startX, startY - TileCounterSize.y);

	//p1
	AnimPathPoints.emplace_back(startX, startY);

	//p2
	AnimPathPoints.emplace_back(startX + (DestinationLeft ? -1.f : 1.f) * TileCounterSize.x, startY + TileCounterSize.y);

	//p3 , p4
	float Dist = glm::distance(glm::vec2(startX, startY), glm::vec2(destX, destY));
	glm::vec2 MidPoint = glm::vec2(AnimPathPoints.back().x + destX, AnimPathPoints.back().y + destY) / 2.f;
	glm::vec2 v = glm::normalize(glm::vec2(destX, destY) - MidPoint);
	glm::vec2 Int0 = glm::vec2(AnimPathPoints.back().x, AnimPathPoints.back().y) + v * (Dist / 5.f);
	glm::vec2 Int1 = glm::vec2(Int0.x, Int0.y) + v * (Dist / 5.f);
	float Mul = Dist / 3.f;
	
	AnimPathPoints.emplace_back(Int0.x + ((DestinationLeft ? 1.f : -1.f) * (v.y * Mul)), Int0.y + ((DestinationLeft ? -1.f : 1.f) * (v.x * Mul)));
	AnimPathPoints.emplace_back(Int1.x + ((DestinationLeft ? 1.f : -1.f) * (v.y * Mul)), Int1.y + ((DestinationLeft ? -1.f : 1.f) * (v.x * Mul)));

	//p5
	AnimPathPoints.emplace_back(destX, destY);

	//p6
	AnimPathPoints.emplace_back(destX - TileCounterSize.x, destY);

	m_AnimationPath.CreatePath(AnimPathPoints, 2);
}
