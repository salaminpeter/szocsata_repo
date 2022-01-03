#pragma once

#include <chrono>

#include "Computer.h"
#include "LetterPool.h"
#include "GameBoard.h"
#include "BinaryBoolList.h"
#include "DataBase.h"
#include "WordTree.h"
#include "glm\gtc\matrix_transform.hpp"

#define WIN_HEADER_SIZE 31     //ocsmany!!!

#include <sstream>
#include <mutex>

#ifdef PLATFORM_ANDROID
#include <jni.h>
#endif

class CRenderer;
class CUIManager;
class CTimerEventManager;
class CTileAnimationManager;
class CWordAnimationManager;
class CLetterModel;
class CCameraAnimationManager;
class CPlayerLetterAnimationManager;
class CDimmBGAnimationManager;

class CGameManager
{
public:
	
	enum EGameState { NextTurn, WaintingOnAnimation, WaitingForMessageBox, TurnInProgress, GameAboutToEnd, GameEnded, BeginGame, Paused, OnStartGameScreen, OnStartScreen, OnRankingsScreen, None};

	CGameManager();

	~CGameManager()
	{
		delete m_Renderer;
	}

	void AddPlayers(int playerCount, bool addComputer);
	void StartGame();
	void InitBasedOnTileCount();
	void InitPlayers();
	void SetTileCount();
	void InitLetterPool();
	int CalculateScore(const TWordPos& word, std::vector<TWordPos>* crossingWords = nullptr);
	void StartInitRenderer(int surfaceWidth, int surfaceHeight);
	void MiddleInitRender();
	void EndInitRenderer();
	void InitUIManager();
	void ShowCountDown();
	void PositionUIElements();
	void RenderFrame();
	void RenderUI();
	void RenderTileAnimations();
	void RenderPlacedLetterSelections();
	void AddWordSelectionAnimation(const std::vector<TWordPos>& wordPos, bool positive);
	void FinishRenderInit();

	void HandleReleaseEventFromBoardView(int x, int y);
	void HandleReleaseEventFromUIView(int x, int y);
	void HandleReleaseEvent(int x, int y);
	void HandleToucheEvent(int x, int y);
	void HandleDragEvent(int x, int y);
	void HandleDragFromBoardView(int x, int y);
	void HandleDragFromUIView(int x, int y);
	void HandleZoomEvent(float dist, int origoX, int origoY);
	void HandleZoomEvent(float dist);
	void HandleZoomEndEvent();
    void HandleMultyDragEvent(int x0, int y0, int x1, int y1);

	void StartPlayerTurn(CPlayer* player);
	void StartComputerturn();
	bool EndComputerTurn();
	bool EndPlayerTurn(bool stillHaveTime = true);
	void NextPlayerTurn();
	void HandlePlayerPass();
	void UndoLastStep();
	void UndoAllSteps();
	void UndoStep(size_t idx);
	void UndoStepAtPos(int x, int y);
	void RemovePlacedLetterSelAtPos(int x, int y);
	int GetPlayerStepIdxAtPos(int x, int y);
	void DealCurrPlayerLetters();
	void PlayerLetterReleased(unsigned letterIdx);
	void EndGameAfterLastPass();
	void GameLoop();
	void SetGameState(int state);
	void DealComputerLettersEvent();
	int PlayerWordHorizontal();
	EGameState GetGameState();
	CLetterModel* AddLetterToBoard(int x, int y, wchar_t c, float height);
	void AddWordSelectionAnimationForComputer();
	void UpdatePlayerScores();
	std::wstring GetNextPlayerName();
	wchar_t GetChOnBoard(int x, int y) { return m_GameBoard(x, y).m_Char; }
	int GetDifficulty();
	bool AllPlayersPassed();
	bool GetPlayerProperties(size_t idx, std::wstring& name, int& score, glm::vec3& color);
	bool GameScreenActive();
	void ShowNextPlayerPopup();
	bool TileAnimationFinished();
	bool PlayerLetterAnimationFinished();
	void StartDimmingAnimation();
	bool SelectionPosIllegal(int x, int y);
	void CheckAndUpdateTime(double& timeFromStart, double& timeFromPrev);

	glm::ivec2 GetScorePanelSize();
	float GetLetterSize();
	std::wstring GetTimeStr(int msec);

	std::wstring GetWordAtPos(bool horizontal, int& x, int& y);

	CWordTree::TNode* WordTreeRoot(wchar_t c) {return m_DataBase.GetWordTreeRoot(c);}
	TField& Board(int x, int y) {return m_GameBoard(x, y);}
	std::wstring CurrentPlayerName() {return m_CurrentPlayer ? m_CurrentPlayer->GetName() : L"";}
	int GetPlayerCount() {return m_Players.size();}
	void SetLastTouchOnBoardView(bool onBoardView) { m_LastTouchOnBoardView = onBoardView; }
	void SetLastTouchPos(int x, int y) { m_LastTouchX = x; m_LastTouchY = y; }
	CPlayer* GetCurrentPlayer() {return m_CurrentPlayer;}

#ifdef PLATFORM_ANDROID
	void SetRendererObject(jobject obj) {
	    m_RendererObject = obj;
	}
#endif

	CRenderer* GetRenderer() { return m_Renderer; }
	CUIManager* GetUIManager() { return m_UIManager; }
	CTimerEventManager* GetTimerEventManager() {return m_TimerEventManager;}
	int GetLetterPoolCount() {return m_LetterPool.GetRemainingLetterCount(); }
	void AddPlacedLetterSelection(int x, int y) {m_PlacedLetterSelections.push_back(glm::ivec2(x, y));}

	glm::vec2 GetViewPosition(const char* viewId);
	bool PositionOnBoardView(int x, int y);
	
	bool HasEmptyFieldInWord(int& min, int& max);
	glm::ivec2 GetSelectionPosition();
	glm::vec2 GetSelectControlsize();
	void SetDimmPanelOpacity(float opacity);

	void PauseGameEvent();
	void EndPlayerTurnEvent();
	void TopViewEvent();
	void GoToStartGameScrEvent();

	int frames = 0;
	std::chrono::high_resolution_clock::time_point LastRenderTime;

	//==============================================================================
	CGameBoard CompGameBoard;
	std::wstring CompLetters;
	void UndoComp()
	{
		m_GameBoard = CompGameBoard;
		m_Computer->m_Letters = CompLetters;
		m_Computer->ResetUsedLetters();
		StartComputerturn();
	}
	//==============================================================================

private:

	std::vector<CPlayer*> m_Players;
	CComputer* m_Computer = nullptr; 

	std::vector<TPlayerStep> m_PlayerSteps;
	std::vector<glm::ivec2> m_PlacedLetterSelections;

	CLetterPool m_LetterPool;
	CGameBoard m_GameBoard;
	CGameBoard m_TmpGameBoard;
	CDataBase m_DataBase;
	CRenderer* m_Renderer = nullptr;

	CUIManager* m_UIManager;
	CTimerEventManager* m_TimerEventManager;
	CTileAnimationManager* m_TileAnimations;
	CWordAnimationManager* m_WordAnimation;
	CCameraAnimationManager* m_CameraAnimationManager;
	CPlayerLetterAnimationManager* m_PlayerLetterAnimationManager;
	CDimmBGAnimationManager* m_DimmBGAnimationManager;

	public: //TODO
	bool m_Dragged = false;
	bool m_LastTouchOnBoardView;
	int m_LastTouchX;
	int m_LastTouchY;
	int m_TouchX = -1;
	int m_TouchY;
	int m_PlayerStepIdxUndo;
	int m_PlacedLetterTouchX = -1;
	int m_PlacedLetterTouchY;
	int m_SurfaceWidth;
	int m_SurfaceHeigh;
	bool m_NextPlayerPopupShown = false;

#ifdef PLATFORM_ANDROID
	JavaVM* m_JavaVM;
	jobject m_RendererObject = nullptr;
#endif

	EGameState m_GameState = EGameState::None;
	EGameState m_PrevGameState = EGameState::None;

	std::mutex m_GameStateLock;
	std::mutex m_PlayerPopupLock;

	CPlayer* m_CurrentPlayer = nullptr;

	int m_ComputerWordIdx = -1;

	int m_LastTurnTimeChanged = 0;
	int m_TurnTimeStart = 0;

	bool m_GamePaused = false;
};

