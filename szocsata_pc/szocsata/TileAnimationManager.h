#pragma once

#include <vector>
#include "glm\gtc\matrix_transform.hpp"

#include "Timer.h"


class CModel;
class CRoundedSquarePositionData;
class CTimerEventManager;
class CGameManager;
class CUIManager;

class CTileAnimationManager
{
public:

	enum EAnimationType {WordSelectionSuccess, WordSelectionFail, CursorSelectionOk, CursorSelectionFail, Last};

	struct TAnimation
	{
		std::vector<glm::ivec2> m_TilePositions;
		glm::vec3 m_Color;
		glm::vec3 m_StartColor;
		glm::vec3 m_DestColor;
		EAnimationType m_AnimationType;
		int m_PassedTime = 0;
		bool m_HandleFinishEvent = false;
		bool m_IgnoreFinishEvent;

		TAnimation(EAnimationType animType) : m_AnimationType(animType) {}
	};

	CTileAnimationManager() = default;
	CTileAnimationManager(CTimerEventManager* timerEventMgr, CGameManager* gameManager);

	~CTileAnimationManager();

	void AddTile(int x, int y, EAnimationType animType, bool addSelectionOnly);
	void StartAnimation(EAnimationType animType);
	void StartAnimation();
	void UpdateColorEvent(double& timeFromStart, double& timeFromPrev);
	void SaveState(std::ofstream& fileStream);
	void LoadState(std::ifstream& fileStream);
	void Reset();
    int GetSelectionType(EAnimationType animType);
    bool Empty(EAnimationType animType);
    void StopAnimation(EAnimationType animType);
    glm::ivec2 GetSelectionPos(EAnimationType animType, size_t pos);

private:

	CTimerEventManager* m_TimerEventManager;
	CGameManager* m_GameManager;
	std::mutex m_TileAnimLock;

	std::vector<TAnimation> m_Animations;

    const int m_WordAnimLength = 1500; //config!!
    const int m_CursorAnimLength = 800; //config!!
};