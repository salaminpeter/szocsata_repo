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


class CUIManager
{
public:

	CUIManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : 
		m_GameManager(gameManager),
		m_TimerEventManager(timerEventManager)
	{}

	//TODO destruktor delete !!!!!!!!!!!!!!!

	void InitUIElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData);
	void PositionUIElements();
	void InitRankingsPanel();

	void AddButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	void AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	void AddText(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	void AddPlayerLetters(const wchar_t* playerId, const wchar_t* letters, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const char* viewID);
	void PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size);

	CUIPlayerLetters* GetPlayerLetters(const std::wstring& playerID);
	CUIPlayerLetters* GetPlayerLetters(size_t playerLetterIdx);

	void HandleTouchEvent(int x, int y);

	void SetText(const wchar_t* id, const wchar_t* text);
	void ShowMessageBox(int type, const wchar_t* text);

	void RenderUI();
	void RenderTexts();
	void RenderButtons();
	void RenderSelectControls();
	void RenderPlayerLetters(const wchar_t* id);
	void RenderTileCounter();
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

private:

	CUIText* GetText(const wchar_t* id) const;
	bool IsGameButton(const CUIButton* button) const;

public:

	CGameManager* m_GameManager;
	CTimerEventManager* m_TimerEventManager;

	std::vector<CUIButton*> m_UIButtons;  //TODO emplace !!!
	std::vector<CUIText*> m_UITexts;
	std::vector<CUIPlayerLetters*> m_UIPlayerLetters;
	std::vector<CUISelectControl*> m_SelectControls;

	CUIElement* m_RootStartScreen;
	CUIElement* m_RootGameScreen;
	CUIElement* m_RootGameEndScreen;

	CUITileCounter* m_UITileCounter;
	CUIMessageBox* m_MessageBoxOk;
	CUIToast* m_Toast;
	CUIRankingsPanel* m_RankingsPanel;
	CGridLayout* m_PlayerLettersLayout;
	CGridLayout* m_ButtonsLayout;

	bool m_GameButtonsDisabled = false;
	bool m_StartScreenActive = true;
};
