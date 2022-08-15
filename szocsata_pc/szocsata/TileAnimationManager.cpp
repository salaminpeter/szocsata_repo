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

void CTileAnimationManager::StartAnimation(bool positive)
{
	if (positive)
		m_Color = glm::vec4(0.4f, 1.f, 0.4f, 0.f);
	else
		m_Color = glm::vec4(1.f, 0.f, 0.f, 0.f);

	m_TimerEventManager->AddTimerEvent(this, &CTileAnimationManager::UpdateColorEvent, positive ? &CTileAnimationManager::AnimFinishedEvent : nullptr, "tile_animation");
	m_TimerEventManager->StartTimer("tile_animation");
}

void CTileAnimationManager::Reset() 
{ 
	m_TimerEventManager->StopTimer("tile_animation");
	m_TilePositions.clear();
}

void CTileAnimationManager::AnimFinishedEvent()
{
	m_GameManager->SetTaskFinished("finish_word_selection_animation_task");
}

void CTileAnimationManager::UpdateColorEvent(double& timeFromStart, double& timeFromPrev)
{
	bool EndAnimation = timeFromStart > m_AnimLength;

	timeFromStart = timeFromStart > m_AnimLength ? m_AnimLength : timeFromStart;
	m_Color.a = std::sin(glm::radians((timeFromStart / m_AnimLength)  * 180.0)) * 0.5;

	if (EndAnimation)
	{
		m_TimerEventManager->StopTimer("tile_animation");
		const std::lock_guard<std::recursive_mutex> lock(m_GameManager->GetRenderer()->GetRenderLock());
		m_TilePositions.clear();
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

