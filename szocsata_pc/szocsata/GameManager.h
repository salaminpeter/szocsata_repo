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

class CGameManager
{
public:
	
	enum EGameState { NextTurn, WaintingOnAnimation, WaitingForMessageBox, TurnInProgress, GameAboutToEnd, GameEnded, BeginGame, OnStartGameScreen, OnStartScreen, OnRankingsScreen, None};

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
	void EndInitRenderer();
	void InitUIManager();
	void PositionUIElements();
	void RenderFrame();
	void RenderUI();
	void RenderTileAnimations();
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
	bool EndPlayerTurn(bool allowPass = true, bool allowNegativeSelection = true);
	void NextPlayerTurn();
	void HandlePlayerPass();
	void UndoLastStep();
	void UndoAllSteps();
	void DealCurrPlayerLetters();
	void PlayerLetterClicked(unsigned letterIdx);
	void EndGameAfterLastPass();
	void GameLoop();
	void SetGameState(int state);
	void DealComputerLettersEvent();
	EGameState GetGameState();
	CLetterModel* AddLetterToBoard(int x, int y, wchar_t c, float height);
	void AddWordSelectionAnimationForComputer();
	void UpdatePlayerScores();
	std::wstring GetNextPlayerName();
	wchar_t GetChOnBoard(int x, int y) { return m_GameBoard(x, y).m_Char; }
	int GetDifficulty();
	bool AllPlayersPassed();
	bool GetPlayerNameScore(size_t idx, std::wstring& name, int& score);
	bool GameScreenActive();
	void ShowNextPlayerPopup();
	bool TileAnimationFinished();
	bool PlayerLetterAnimationFinished(bool checkMsgBox = false);
	bool SelectionPosIllegal(int x, int y);
	void CheckAndUpdateTime(double& timeFromStart, double& timeFromPrev);
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

	glm::vec2 GetViewPosition(const char* viewId);
	bool PositionOnBoardView(int x, int y);

	void EndPlayerTurnEvent();
	void BackSpaceEvent();
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

	public: //TODO
	bool m_Dragged = false;
	bool m_LastTouchOnBoardView;
	int m_LastTouchX;
	int m_LastTouchY;
	int m_TouchX = -1;
	int m_TouchY;
	int m_SurfaceWidth;
	int m_SurfaceHeigh;
	bool m_NextPlayerPopupShown = false;

#ifdef PLATFORM_ANDROID
	JavaVM* m_JavaVM;
	jobject m_RendererObject = nullptr;
#endif

	EGameState m_GameState = EGameState::None;

	std::mutex m_GameStateLock;
	CPlayer* m_CurrentPlayer = nullptr;

	int m_FirstPlayerLetterX = -1;
	int m_FirstPlayerLetterY = -1;
	int m_SecondPlayerLetterX = -1;
	int m_SecondPlayerLetterY = -1;

	int m_ComputerWordIdx = -1;

	int m_LastTurnTimeChanged = 0;
	int m_TurnTimeStart = 0;
};

