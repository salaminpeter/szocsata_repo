#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GLES3\gl3.h>

#include "TileAnimationManager.h"
#include "UIManager.h"
#include "Config.h"
#include "SquareModelData.h"
#include "Renderer.h"
#include "Timer.h"
#include "TimerEventManager.h"
#include "GameManager.h"
#include "UIMessageBox.h"
#include "SelectionStore.h"

#include <iostream>
#include <fstream>


CTileAnimationManager::CTileAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager) :
	m_TimerEventManager(timerEventMgr),
	m_GameManager(gameManager)
{
	m_TimerEventManager->AddTimerEvent(this, &CTileAnimationManager::UpdateColorEvent, nullptr, "tile_animation");
	m_Animations.reserve(EAnimationType::Last);

	for (int i = 0; i < EAnimationType::Last; ++i)
		m_Animations.emplace_back(static_cast<EAnimationType>(i));
}

CTileAnimationManager::~CTileAnimationManager()
{
	m_Animations.clear();
	m_TimerEventManager->RemoveTimer("tile_animation");
}

void CTileAnimationManager::AddTile(int x, int y, EAnimationType animType, bool addSelectionOnly)
{
	glm::ivec2 TilePos(x, y);
	std::vector<glm::ivec2>& TilePositions = m_Animations[animType].m_TilePositions;

	if (addSelectionOnly || std::find(TilePositions.begin(), TilePositions.end(), TilePos) == TilePositions.end())
	{
		if (!addSelectionOnly)
			TilePositions.push_back(glm::ivec2(x, y));

		CSelectionStore* SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();
		glm::vec3 ModifyColor(1,1,1);
		std::string Id;

		if (animType == CTileAnimationManager::WordSelectionFail || animType == CTileAnimationManager::WordSelectionSuccess)
			Id = "word_selection";
		else if (animType == CTileAnimationManager::CursorSelectionOk || animType == CTileAnimationManager::CursorSelectionFail)
			Id = "board_selection";

		SelectionStore->AddSelection(static_cast<CSelectionStore::ESelectionType>(GetSelectionType(animType)), x, y, Id.c_str(), &ModifyColor);
	}
}

bool CTileAnimationManager::Empty(EAnimationType animType) 
{
	return (m_Animations[animType].m_TilePositions.size() == 0);
}

int CTileAnimationManager::GetSelectionType(EAnimationType animType)
{
    switch (animType)
    {
        case CTileAnimationManager::WordSelectionSuccess :
            return CSelectionStore::SuccessSelection;
        case CTileAnimationManager::WordSelectionFail :
            return CSelectionStore::FailSelection;
        case CTileAnimationManager::CursorSelectionOk :
            return CSelectionStore::BoardSelectionOk;
        case CTileAnimationManager::CursorSelectionFail :
            return CSelectionStore::BoardSelectionFail;
        default: return CSelectionStore::BoardSelectionOk;
    }
}

void CTileAnimationManager::StartAnimation()
{
	bool HasAnimation = false;

	for (int i = 0; i < EAnimationType::Last; ++i)
	{
		if (m_Animations[i].m_TilePositions.size() == 0)
			continue;

		m_Animations[i].m_IgnoreFinishEvent = !m_Animations[i].m_HandleFinishEvent;
		HasAnimation = true;
	}

	if (HasAnimation)
		m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::StartAnimation(CTileAnimationManager::EAnimationType animType)
{
	CSelectionStore* SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();

	m_Animations[animType].m_StartColor = glm::vec3(100,100,100);
	m_Animations[animType].m_DestColor = SelectionStore->GetColorModifyer(static_cast<CSelectionStore::ESelectionType>(GetSelectionType(animType)));
	m_Animations[animType].m_DestColor *= 100;

	m_Animations[animType].m_IgnoreFinishEvent = animType == CTileAnimationManager::WordSelectionFail;
	m_Animations[animType].m_HandleFinishEvent = animType == CTileAnimationManager::WordSelectionSuccess;;
	m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::Reset() 
{ 
	const std::lock_guard<std::mutex> lock(m_TileAnimLock);
	m_TimerEventManager->PauseTimer("tile_animation");

	for (auto Animations : m_Animations)
		Animations.m_TilePositions.clear();
}

void CTileAnimationManager::SaveState(std::ofstream& fileStream)
{
	for (int i = 0; i < EAnimationType::Last; ++i)
	{
		size_t TileAnimCount = m_Animations[i].m_TilePositions.size();
		m_TimerEventManager->PauseTimer("tile_animation");
		fileStream.write((char *) &TileAnimCount, sizeof(size_t));

		if (TileAnimCount != 0) {
			fileStream.write((char *) &m_Animations[i].m_PassedTime, sizeof(int));
			fileStream.write((char *) &m_Animations[i].m_Color.r, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_Color.g, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_Color.b, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_StartColor.r, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_StartColor.g, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_StartColor.b, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_DestColor.r, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_DestColor.g, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_DestColor.b, sizeof(float));
			fileStream.write((char *) &m_Animations[i].m_HandleFinishEvent, sizeof(bool));
			fileStream.write((char *) &m_Animations[i].m_AnimationType, sizeof(int));
		}

		for (auto TilePos : m_Animations[i].m_TilePositions) {
			fileStream.write((char *) &TilePos.x, sizeof(int));
			fileStream.write((char *) &TilePos.y, sizeof(int));
		}
	}
}

void CTileAnimationManager::LoadState(std::ifstream& fileStream)
{
	m_TimerEventManager->PauseTimer("tile_animation");

	for (int i = 0; i < EAnimationType::Last; ++i) {
		size_t TileAnimCount;
		fileStream.read((char *) &TileAnimCount, sizeof(size_t));

		if (TileAnimCount != 0) {
			fileStream.read((char *) &m_Animations[i].m_PassedTime, sizeof(int));
			fileStream.read((char *) &m_Animations[i].m_Color.r, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_Color.g, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_Color.b, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_StartColor.r, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_StartColor.g, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_StartColor.b, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_DestColor.r, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_DestColor.g, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_DestColor.b, sizeof(float));
			fileStream.read((char *) &m_Animations[i].m_HandleFinishEvent, sizeof(bool));
			fileStream.read((char *) &m_Animations[i].m_AnimationType, sizeof(int));
		}

		for (int j = 0; j < TileAnimCount; ++j) {
			int x, y;
			fileStream.read((char *) &x, sizeof(int));
			fileStream.read((char *) &y, sizeof(int));
			AddTile(x, y, static_cast<EAnimationType>(i), false);
		}
	}
}

void CTileAnimationManager::StopAnimation(EAnimationType animType)
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());
	{
		const std::lock_guard<std::mutex> lock(m_TileAnimLock);
		{
//			const std::lock_guard<std::mutex> lock(m_GameManager->GetRenderer()->GetRenderLock());
//			{
				m_Animations[animType].m_TilePositions.clear();
				m_Animations[animType].m_PassedTime = 0;
//			}
		}
	}
}

glm::ivec2 CTileAnimationManager::GetSelectionPos(EAnimationType animType, size_t pos)
{
	if (pos >= m_Animations[animType].m_TilePositions.size())
		return glm::ivec2(-1, -1);

	return m_Animations[animType].m_TilePositions[pos];
}

void CTileAnimationManager::UpdateColorEvent(double& timeFromStart, double& timeFromPrev)
{
	for (int i = 0; i < EAnimationType::Last; ++i)
	{
		if (m_Animations[i].m_TilePositions.size() == 0)
			continue;

		m_Animations[i].m_PassedTime += timeFromPrev;
		int AnimLength = 0;

		if (i == EAnimationType::WordSelectionFail || i == EAnimationType::WordSelectionSuccess)
			AnimLength = m_WordAnimLength;
		else if (i == EAnimationType::CursorSelectionFail || i == EAnimationType::CursorSelectionOk)
			AnimLength = m_CursorAnimLength;

		bool EndAnimation = (m_Animations[i].m_PassedTime > AnimLength);

		m_Animations[i].m_PassedTime = EndAnimation ? AnimLength : m_Animations[i].m_PassedTime;

		glm::vec3 ColorDiff = (m_Animations[i].m_DestColor - m_Animations[i].m_StartColor) * static_cast<float>(std::sin(glm::radians((static_cast<double>(m_Animations[i].m_PassedTime) / AnimLength) * 180.0)));
		m_Animations[i].m_Color.r = (m_Animations[i].m_StartColor.r + ColorDiff.r) / 100.f;
		m_Animations[i].m_Color.g = (m_Animations[i].m_StartColor.g + ColorDiff.g) / 100.f;
		m_Animations[i].m_Color.b = (m_Animations[i].m_StartColor.b + ColorDiff.b) / 100.f;

		CSelectionStore *SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();

		if (EndAnimation) {
			const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());
			{
				CSelectionStore *SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();

				//finish animation
				if (i == CTileAnimationManager::WordSelectionSuccess || i == CTileAnimationManager::WordSelectionFail) {
					SelectionStore->ClearSelections(static_cast<CSelectionStore::ESelectionType>(GetSelectionType(static_cast<EAnimationType>(i))));
					m_Animations[i].m_PassedTime = 0;
					m_TimerEventManager->FinishTimer("tile_animation");

					const std::lock_guard<std::mutex> lock(m_TileAnimLock);
					{
						const std::lock_guard<std::mutex> lock(m_GameManager->GetRenderer()->GetRenderLock());
						{
							m_Animations[i].m_TilePositions.clear();
							m_Animations[i].m_PassedTime = 0;

							if (!m_Animations[i].m_IgnoreFinishEvent && i == EAnimationType::WordSelectionSuccess)
								m_GameManager->SetTaskFinished("finish_word_selection_animation_task");
							else if (i != EAnimationType::WordSelectionSuccess)
								m_Animations[i].m_IgnoreFinishEvent = false;
						}
					}
				}
					//restart animation
				else
					m_Animations[i].m_PassedTime = 0;
			}
		}

		SelectionStore->SetModifyColor(static_cast<CSelectionStore::ESelectionType>(GetSelectionType(static_cast<EAnimationType>(i))), m_Animations[i].m_Color);
	}
}
