#pragma once

#include "UIText.h"

#include <vector>
#include <memory>
#include <mutex>
#include <glm/glm.hpp>

class CUIButton;
class CUIIconTextButton;
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
class CUIPlayerLetterPanel;
class CPlayer;
class CUIVerticalLayout;

class CUIManager
{
public:

	CUIManager(CGameManager* gameManager, CTimerEventManager* timerEventManager) : 
		m_GameManager(gameManager),
		m_TimerEventManager(timerEventManager)
	{}

	~CUIManager();

	glm::vec2 GetElemSize(const wchar_t* id);

	glm::vec2 GetTileCounterPos();
	glm::vec2 GetTileCounterSize();

	CUIButton* AddButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id, const char* shaderID = "textured"); 
	CUIIconTextButton* AddIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const char* iconIextureID, const wchar_t* id, const char* shaderID = "textured", float iconSize = 0.65f, float iconWHRatio = 1.f, float charSize = 0.35f, float padding = 0.f, CUIText::ETextAlign align = CUIText::Left);
	CUISelectControl* AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id, const wchar_t* description);
	CUIText* AddText(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float fontHeight, const char* ViewID, const char* textureID, const wchar_t* id, float r = 0.92f, float g = 0.92f, float b = 0.92f);
	CUIPlayerLetters* AddPlayerLetters(CPlayer* player, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, bool addLetters = true);
	CUIElement* GetUIElement(const wchar_t* id);
	
	CUIPlayerLetters* GetPlayerLetters(const std::wstring& playerID);
	CUIPlayerLetters* GetPlayerLetters(size_t idx);

	void HandleTouchEvent(int x, int y);
	void HandleReleaseEvent(int x, int y);
	void HandleDragEvent(int x, int y);
	bool HandleDoubleClickEvent(int x, int y);


	void SetText(const wchar_t* id, const wchar_t* text);
	void ShowMessageBox(int type, const wchar_t* text);
	void CloseMessageBox();
	void ShowToast(const wchar_t* text);

	void RenderUI();
	void RenderMessageBox();
	void RenderDraggedLetter();

	void SetTileCounterValue(unsigned count);
	int GetTileCounterValue();

	void SetCurrentPlayerName(const wchar_t* playerName, float r, float g, float b);
	void EnableGameButtons(bool enable);

	void CloseToast(double& timeFromStart, double& timeFromPrev);

	int GetDifficulty();
	int GetBoardSize();
	int GetPlayerCount();
	int GetTimeLimit();
	int GetTimeLimitIdx();
	int Get3DDetail();
	int GetLightQuality();
	int GetFPSLimit();
	int ComputerOpponentEnabledIdx();

	float GetLetterSize();
	void ShowPlayerLetters(bool show, const wchar_t* playerId);

	void UpdateRankingsPanel();
	void UpdateScorePanel();
	void InitScorePanel();
	void InitFont();
	glm::ivec2 GetScorePanelSize();

	void ShowSettingsPanel(int panelIdx);
	void ClearUIElements();
	void ClearGameScreenUIElements();

	void SetDimmPanelOpacity(float opacity);

	CUIElement* GetActiveScreenUIRoot();

	void SetDraggedPlayerLetter(bool letterDragged, unsigned letterIdx, const glm::vec2& letterTexPos, const glm::vec2& startDragPos, bool disable = false);

	void SetRemainingTimeStr(const wchar_t* timeStr);

	void InitBaseElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4);
	void InitStartScreenElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4);

	void InitMainScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8);
	void InitStartGameScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8);
	void InitGameScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8);
	void InitRankingsScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8);
	void InitSettingsScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData16x6);
	void SetScorePanelLayoutBox();

	std::recursive_mutex& GetUILock() { return m_UILock; }


private:

	CUIText* GetText(const wchar_t* id) const;
	bool IsGameButton(const CUIButton* button) const;
	glm::ivec2 GetSizeByArea(float areaRatio, float whRatio, float parentArea, float maxWidth);

public: //TODO

	CGameManager* m_GameManager;
	CTimerEventManager* m_TimerEventManager;

	std::vector<CUIElement*> m_UIRoots;

	CUIElement* m_RootStartScreen = nullptr;
	CUIElement* m_RootStartGameScreen = nullptr;
	CUIElement* m_RootGameScreen = nullptr;
	CUIElement* m_RootDraggedLetterScreen = nullptr;
	CUIElement* m_RootGameEndScreen = nullptr;
	CUIElement* m_RootSettingsScreen = nullptr;

	CUITileCounter* m_UITileCounter;
	CUIMessageBox* m_MessageBoxOk;
	CUIMessageBox* m_MessageBoxResumeGame;
	CUIToast* m_Toast;
	CUIScorePanel* m_ScorePanel;
	CUIRankingsPanel* m_RankingsPanel;
	CUIPlayerLetterPanel* m_PlayerLetterPanel;
	CUIPanel* m_UIScreenPanel;
	CUIPanel* m_GameScreenPanel;
	CUIPanel* m_DimmPanel;

	std::map<std::string, glm::ivec2> m_UIElemSize;

	CUIVerticalLayout* m_MainScreenBtnLayout;

	glm::vec2 m_LastDraggedPlayerLetterPos;

	std::recursive_mutex m_UILock;

	size_t m_DraggedPlayerLetterIdx;
	bool m_PlayerLetterDragged = false;
	bool m_UIInitialized = false;
};
