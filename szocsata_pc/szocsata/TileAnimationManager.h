#pragma once

#include <vector>
#include "glm\gtc\matrix_transform.hpp"

#include "Timer.h"


class CSelectionModel;
class CModel;
class CRoundedSquarePositionData;
class CTimerEventManager;
class CGameManager;

class CTileAnimationManager
{
public:
	
	CTileAnimationManager() = default;
	CTileAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager);

	void StarDataQuery() { m_QueryIndex = 0; }
	glm::vec4 GetColor() { return m_Color; }

	void AddTile(int x, int y);
	bool GetData(glm::ivec2& position, bool& lastTile);
	void StartAnimation(bool positive);

	void UpdateColorEvent(double& timeFromStart, double& timeFromPrev);
	void AnimFinishedEvent();


private:

	std::vector<glm::ivec2> m_TilePositions;
	glm::vec4 m_Color;
	size_t m_QueryIndex;

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;

	const int m_AnimLength = 1500; //config!!
};