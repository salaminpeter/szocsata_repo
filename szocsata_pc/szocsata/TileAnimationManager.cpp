#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GLES3\gl3.h>

#include "TileAnimationManager.h"
#include "UIManager.h"
#include "SelectionModel.h"
#include "Config.h"
#include "SquareModelData.h"
#include "Renderer.h"
#include "Timer.h"
#include "TimerEventManager.h"
#include "GameManager.h"
#include "UIMessageBox.h"

#include <iostream>
#include <fstream>


CTileAnimationManager::CTileAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager) :
	m_TimerEventManager(timerEventMgr),
	m_GameManager(gameManager)
{
}

CTileAnimationManager::~CTileAnimationManager()
{
	m_TimerEventManager->StopTimer("tile_animation");
}

void CTileAnimationManager::AddTile(int x, int y)
{
	glm::ivec2 TilePos(x, y);

	if (std::find(m_TilePositions.begin(), m_TilePositions.end(), TilePos) == m_TilePositions.end())
		m_TilePositions.push_back(glm::ivec2(x, y));
}

void CTileAnimationManager::StartAnimation()
{
	m_TimerEventManager->AddTimerEvent(this, &CTileAnimationManager::UpdateColorEvent, m_HandleFinishEvent ? &CTileAnimationManager::AnimFinishedEvent : nullptr, "tile_animation");
	m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::StartAnimation(bool positive)
{
	if (positive)
		m_Color = glm::vec4(0.4f, 1.f, 0.4f, 0.f);
	else
		m_Color = glm::vec4(1.f, 0.f, 0.f, 0.f);
	
	m_HandleFinishEvent = positive;
	m_TimerEventManager->AddTimerEvent(this, &CTileAnimationManager::UpdateColorEvent, positive ? &CTileAnimationManager::AnimFinishedEvent : nullptr, "tile_animation");
	m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::Reset() 
{ 
	const std::lock_guard<std::mutex> lock(m_TileAnimLock);
	m_TimerEventManager->StopTimer("tile_animation");
	m_TilePositions.clear();
}

void CTileAnimationManager::SaveState(std::ofstream& fileStream)
{
	const std::lock_guard<std::mutex> lock(m_TileAnimLock);

	size_t TileAnimCount = m_TilePositions.size();
	m_TimerEventManager->PauseTimer("tile_animation");
	fileStream.write((char *)&TileAnimCount, sizeof(size_t));

	if (TileAnimCount != 0)
	{
		fileStream.write((char *)&m_PassedTime, sizeof(int));
		fileStream.write((char *)&m_Color.r, sizeof(float));
		fileStream.write((char *)&m_Color.g, sizeof(float));
		fileStream.write((char *)&m_Color.b, sizeof(float));
		fileStream.write((char *)&m_HandleFinishEvent, sizeof(bool));
	}

	for (auto TilePos : m_TilePositions)
	{
		fileStream.write((char *)&TilePos.x, sizeof(int));
		fileStream.write((char *)&TilePos.y, sizeof(int));
	}
}

void CTileAnimationManager::LoadState(std::ifstream& fileStream)
{
	size_t TileAnimCount;
	fileStream.read((char *)&TileAnimCount, sizeof(size_t));

	if (TileAnimCount != 0)
	{
		fileStream.read((char *)&m_PassedTime, sizeof(int));
		fileStream.read((char *)&m_Color.r, sizeof(float));
		fileStream.read((char *)&m_Color.g, sizeof(float));
		fileStream.read((char *)&m_Color.b, sizeof(float));
		fileStream.read((char *)&m_HandleFinishEvent, sizeof(bool));
	}

	for (int i = 0; i < TileAnimCount; ++i)
	{
		int x, y;
		fileStream.read((char *)&x, sizeof(int));
		fileStream.read((char *)&y, sizeof(int));
		AddTile(x, y);
	}
}


void CTileAnimationManager::AnimFinishedEvent()
{
	m_GameManager->SetTaskFinished("finish_word_selection_animation_task");
}

void CTileAnimationManager::UpdateColorEvent(double& timeFromStart, double& timeFromPrev)
{
	if (m_PassedTime == 0)
		m_PassedTime = timeFromStart;
	else
		m_PassedTime += timeFromPrev;

	bool EndAnimation = m_PassedTime > m_AnimLength;

	m_PassedTime = m_PassedTime > m_AnimLength ? m_AnimLength : m_PassedTime;
	m_Color.a = std::sin(glm::radians((static_cast<double>(m_PassedTime) / m_AnimLength)  * 180.0)) * 0.5;

	if (EndAnimation)
	{
		m_TimerEventManager->StopTimer("tile_animation");
		const std::lock_guard<std::mutex> lock1(m_TileAnimLock);
		const std::lock_guard<std::recursive_mutex> lock(m_GameManager->GetRenderer()->GetRenderLock());
		m_TilePositions.clear();
		m_PassedTime = 0;
	}
}

bool CTileAnimationManager::GetData(glm::ivec2& position, bool& lastTile)
{
	if (m_QueryIndex >= m_TilePositions.size()) //TODO hogy lehet nagyobb ?????? kress!!!
		return false;

	position = m_TilePositions[m_QueryIndex];
	m_QueryIndex++;
	lastTile = m_QueryIndex == m_TilePositions.size();

	return true;
}

