#pragma once

#include <vector>
#include "glm\gtc\matrix_transform.hpp"

#include "Timer.h"


class CSelectionModel;
class CModel;
class CRoundedSquarePositionData;
class CTimerEventManager;
class CGameManager;
class CUIManager;

class CTileAnimationManager
{
public:
	
	CTileAnimationManager() = default;
	CTileAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager);

	~CTileAnimationManager();

	void AddTile(int x, int y, bool positive);
	void StartAnimation(bool positive);
	void StartAnimation();
	void UpdateColorEvent(double& timeFromStart, double& timeFromPrev);
	void AnimFinishedEvent();
	void SaveState(std::ofstream& fileStream);
	void LoadState(std::ifstream& fileStream);
	void Reset();

	void SetUIManager(CUIManager* uiManager) {m_UIManager = uiManager;}
	glm::vec3 GetColor() { return m_Color; }
	bool Empty() {return (m_TilePositions.size() == 0);}

private:

	std::vector<glm::ivec2> m_TilePositions;
	glm::vec3 m_Color;
	glm::vec3 m_StartColor;
	glm::vec3 m_DestColor;

	int m_SelectionType;

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	CUIManager* m_UIManager;
	std::mutex m_TileAnimLock;
	int m_PassedTime = 0;
	bool m_HandleFinishEvent = false;

	const int m_AnimLength = 2550; //config!!
};