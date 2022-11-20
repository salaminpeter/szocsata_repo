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
	m_TimerEventManager->AddTimerEvent(this, &CTileAnimationManager::UpdateColorEvent, &CTileAnimationManager::AnimFinishedEvent, "tile_animation");
}

CTileAnimationManager::~CTileAnimationManager()
{
	m_TimerEventManager->RemoveTimer("tile_animation");
}

void CTileAnimationManager::AddTile(int x, int y, bool positive)
{
	glm::ivec2 TilePos(x, y);

	if (std::find(m_TilePositions.begin(), m_TilePositions.end(), TilePos) == m_TilePositions.end())
	{
		m_TilePositions.push_back(glm::ivec2(x, y));
		CSelectionStore* SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();
		glm::vec3 ModifyColor(1,1,1);
		SelectionStore->AddSelection(positive ? CSelectionStore::SuccessSelection: CSelectionStore::FailSelection, x, y, "success_selection", &ModifyColor);
	}
}

void CTileAnimationManager::StartAnimation()
{
	m_IgnoreFinishEvent = !m_HandleFinishEvent;
	m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::StartAnimation(bool positive)
{
	CSelectionStore* SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();
	m_SelectionType = positive ? CSelectionStore::SuccessSelection : CSelectionStore::FailSelection;

	m_StartColor = glm::vec3(100,100,100);
	m_DestColor = SelectionStore->GetColorModifyer(static_cast<CSelectionStore::ESelectionType>(m_SelectionType));
	m_DestColor *= 100;
	
	m_IgnoreFinishEvent = !positive;
	m_HandleFinishEvent = positive;
	m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::Reset() 
{ 
	const std::lock_guard<std::mutex> lock(m_TileAnimLock);
	m_TimerEventManager->PauseTimer("tile_animation");
	m_TilePositions.clear();
}

void CTileAnimationManager::SaveState(std::ofstream& fileStream)
{
	size_t TileAnimCount = m_TilePositions.size();
	m_TimerEventManager->PauseTimer("tile_animation");
	fileStream.write((char *)&TileAnimCount, sizeof(size_t));

	if (TileAnimCount != 0)
	{
		fileStream.write((char *)&m_PassedTime, sizeof(int));
		fileStream.write((char *)&m_Color.r, sizeof(float));
		fileStream.write((char *)&m_Color.g, sizeof(float));
		fileStream.write((char *)&m_Color.b, sizeof(float));
		fileStream.write((char *)&m_StartColor.r, sizeof(float));
		fileStream.write((char *)&m_StartColor.g, sizeof(float));
		fileStream.write((char *)&m_StartColor.b, sizeof(float));
		fileStream.write((char *)&m_DestColor.r, sizeof(float));
		fileStream.write((char *)&m_DestColor.g, sizeof(float));
		fileStream.write((char *)&m_DestColor.b, sizeof(float));
		fileStream.write((char *)&m_HandleFinishEvent, sizeof(bool));
		fileStream.write((char *)&m_SelectionType, sizeof(int));
	}

	for (auto TilePos : m_TilePositions)
	{
		fileStream.write((char *)&TilePos.x, sizeof(int));
		fileStream.write((char *)&TilePos.y, sizeof(int));
	}
}

void CTileAnimationManager::LoadState(std::ifstream& fileStream)
{
	m_TimerEventManager->PauseTimer("tile_animation");

	size_t TileAnimCount;
	fileStream.read((char *)&TileAnimCount, sizeof(size_t));

	if (TileAnimCount != 0)
	{
		fileStream.read((char *)&m_PassedTime, sizeof(int));
		fileStream.read((char *)&m_Color.r, sizeof(float));
		fileStream.read((char *)&m_Color.g, sizeof(float));
		fileStream.read((char *)&m_Color.b, sizeof(float));
		fileStream.read((char *)&m_StartColor.r, sizeof(float));
		fileStream.read((char *)&m_StartColor.g, sizeof(float));
		fileStream.read((char *)&m_StartColor.b, sizeof(float));
		fileStream.read((char *)&m_DestColor.r, sizeof(float));
		fileStream.read((char *)&m_DestColor.g, sizeof(float));
		fileStream.read((char *)&m_DestColor.b, sizeof(float));
		fileStream.read((char *)&m_HandleFinishEvent, sizeof(bool));
		fileStream.read((char *)&m_SelectionType, sizeof(int));
	}

	for (int i = 0; i < TileAnimCount; ++i)
	{
		int x, y;
		fileStream.read((char *)&x, sizeof(int));
		fileStream.read((char *)&y, sizeof(int));
		AddTile(x, y, true);
	}
}

void CTileAnimationManager::AnimFinishedEvent()
{
	if (!m_IgnoreFinishEvent)
		m_GameManager->SetTaskFinished("finish_word_selection_animation_task");
	else
		m_IgnoreFinishEvent = false;
}

void CTileAnimationManager::UpdateColorEvent(double& timeFromStart, double& timeFromPrev)
{
	m_PassedTime += timeFromPrev;

	bool EndAnimation = m_PassedTime > m_AnimLength;

	m_PassedTime = EndAnimation ? m_AnimLength : m_PassedTime;

	glm::vec3 ColorDiff = (m_DestColor - m_StartColor) * static_cast<float>(std::sin(glm::radians((static_cast<double>(m_PassedTime) / m_AnimLength) * 180.0)));
	m_Color.r = (m_StartColor.r + ColorDiff.r) / 100.f;
	m_Color.g = (m_StartColor.g + ColorDiff.g) / 100.f;
	m_Color.b = (m_StartColor.b + ColorDiff.b) / 100.f;

	CSelectionStore* SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();

	if (EndAnimation)
	{
		const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());
		{
			CSelectionStore* SelectionStore = m_GameManager->GetRenderer()->GetSelectionStore();
			SelectionStore->ClearSelections(static_cast<CSelectionStore::ESelectionType>(m_SelectionType));
			m_PassedTime = 0;
			m_TimerEventManager->FinishTimer("tile_animation");

			const std::lock_guard<std::mutex> lock(m_TileAnimLock);
			{
				const std::lock_guard<std::recursive_mutex> lock(m_GameManager->GetRenderer()->GetRenderLock());
				{
					m_TilePositions.clear();
					m_PassedTime = 0;
				}
			}
		}
	}

	SelectionStore->SetModifyColor(static_cast<CSelectionStore::ESelectionType>(m_SelectionType), m_Color);
}
