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

	void AddTile(int x, int y);
	bool GetData(glm::ivec2& position, bool& lastTile);
	void StartAnimation(bool positive);
	void StartAnimation();
	void UpdateColorEvent(double& timeFromStart, double& timeFromPrev);
	void AnimFinishedEvent();
	void SaveState(std::ofstream& fileStream);
	void LoadState(std::ifstream& fileStream);
	void Reset();

	void SetUIManager(CUIManager* uiManager) {m_UIManager = uiManager;}
	void StarDataQuery() { m_QueryIndex = 0; }
	glm::vec4 GetColor() { return m_Color; }
	bool Finished() {return (m_TilePositions.size() == 0);}

private:

	std::vector<glm::ivec2> m_TilePositions;
	glm::vec4 m_Color;
	size_t m_QueryIndex;

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	CUIManager* m_UIManager;
	std::mutex m_TileAnimLock;
	int m_PassedTime = 0;
	bool m_HandleFinishEvent = false;

	const int m_AnimLength = 1500; //config!!
};