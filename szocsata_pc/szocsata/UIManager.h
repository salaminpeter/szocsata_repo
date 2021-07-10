#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class CUIButton;
class CUIText;
class CUIPlayerLetters;
class CUIPanel;
class CUIElement;
class CSquarePositionData;
class CSquareColorData;
class CModel;
class CGameManager;
class CTimerEventManager;
class CUITileCounter;
class CUIMessageBox;
class CUIToast;
class CGridLayout;
class CUISelectControl;
class CUIRankingsPanel;
class CUIScorePanel;
class CPlayer;


class CUIManager
{
public:

	CUIManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : 
		m_GameManager(gameManager),
		m_TimerEventManager(timerEventManager)
	{}

	~CUIManager();

	void InitUIElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4);
	void PositionUIElements();
	void InitRankingsPanel();

	CUIButton* AddButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	CUISelectControl* AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	CUIText* AddText(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	CUIPlayerLetters* AddPlayerLetters(CPlayer* player, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData);
	
	void PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size);

	CUIPlayerLetters* GetPlayerLetters(const std::wstring& playerID);

	void HandleTouchEvent(int x, int y);
	void HandleReleaseEvent(int x, int y);
	void HandleDragEvent(int x, int y);

	void SetText(const wchar_t* id, const wchar_t* text);
	void ShowMessageBox(int type, const wchar_t* text);
	void ShowToast(const wchar_t* text, bool endGame = false);

	void RenderUI();
	void RenderMessageBox();
	void RenderRankingsPanel();

	void SetTileCounterValue(unsigned count);

	void ActivateStartScreen(bool activate);

	void PositionPlayerLetters(const std::wstring& playerId);
	void PositionGameButtons();

	void EnableGameButtons(bool enable);

	void CloseToast(double& timeFromStart, double& timeFromPrev);

	int GetDifficulty();
	int GetBoardSize();
	int GetPlayerCount();
	bool ComputerOpponentEnabled();

	void UpdateScorePanel();
	void InitScorePanel();

	CUIElement* GetActiveScreenUIRoot();

	void SetDraggedPlayerLetter(bool letterDragged, unsigned letterIdx, const glm::vec2& letterTexPos, const glm::vec2& startDragPos);

private:

	CUIText* GetText(const wchar_t* id) const;
	bool IsGameButton(const CUIButton* button) const;

public: //TODO

	CGameManager* m_GameManager;
	CTimerEventManager* m_TimerEventManager;

	std::vector<CUIElement*> m_UIRoots;

	CUIElement* m_RootStartScreen;
	CUIElement* m_RootStartGameScreen;
	CUIElement* m_RootGameScreen;
	CUIElement* m_RootGameEndScreen;

	CUITileCounter* m_UITileCounter;
	CUIMessageBox* m_MessageBoxOk;
	CUIToast* m_Toast;
	CUIScorePanel* m_ScorePanel;
	CUIRankingsPanel* m_RankingsPanel;
	CGridLayout* m_PlayerLettersLayout;
	CGridLayout* m_ButtonsLayout;

	glm::vec2 m_LastDraggedPlayerLetterPos;

	size_t m_DraggedPlayerLetterIdx;
	bool m_PlayerLetterDragged = false;
	bool m_GameButtonsDisabled = false;
	bool m_StartScreenActive = true;
};
