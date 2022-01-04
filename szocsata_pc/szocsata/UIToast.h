#pragma once

#include "UIMessageBox.h"

class CTimerEventManager;
class CUIManager;
class CGameManager;

class CUIToast : public CUIMessageBox
{
public:

	CUIToast(unsigned timeout, CTimerEventManager* timerEventManager, CUIManager* uiManager, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type, CGameManager* gameManager) :
		CUIMessageBox(positionData, colorData, gridColorData, x, y, w, h, vx, vy, NoButton, gameManager),
		m_TimeOut(timeout),
		m_TimerEventManager(timerEventManager),
		m_UIManager(uiManager)
	{}

	void StartTimer();

	void SetFinishGame(bool finish) { m_EndGameAfterFinished = finish; }
	bool FinishGame() { return m_EndGameAfterFinished; }

private:

	unsigned m_TimeOut;
	CTimerEventManager* m_TimerEventManager;
	CUIManager* m_UIManager;
	bool m_EndGameAfterFinished = false;
};
