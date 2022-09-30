#pragma once

#include <chrono>
#include <algorithm>
#include <queue>

#include "Computer.h"
#include "LetterPool.h"
#include "GameBoard.h"
#include "BinaryBoolList.h"
#include "DataBase.h"
#include "WordTree.h"
#include "GameState.h"
#include "TaskManager.h"
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
class CGameState;
class CGameThread;

class CGameManager
{
public:
	
	enum EGameState { NextTurn, WaintingOnAnimation, WaitingForMessageBox, TurnInProgress, GameEnded, BeginGame, ContinueGame, Paused, OnStartGameScreen, OnStartScreen, OnRankingsScreen, None};

	CGameManager();

	~CGameManager()
	{
		delete m_State;
		delete m_Renderer;
		delete m_UIManager;
		delete m_TileAnimations;
		delete m_WordAnimation;
		delete m_TimerEventManager;
		delete m_GameThread;
		delete m_TaskManager;

		RemovePlayers();
	}

	void ResetToStartScreen();
	void AddPlayers(int playerCount, bool addComputer, bool addLetters = true);
	void RemovePlayers();
	void StartGame(bool resumeGame);
	void InitBasedOnTileCount(bool addLetters);
	void InitPlayers(bool addLetters);
	void SetTileCount();
	void InitLetterPool(bool initLettersCount = true);
	int CalculateScore(const TWordPos& word, std::vector<TWordPos>* crossingWords = nullptr);
	void CreateRenderer(int surfaceWidth, int surfaceHeight);
	void GenerateStartScreenTextures();
	void GenerateGameScreenTextures();
	void InitUIManager();
	void InitStartUIScreens();
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
	bool HandleDoubleClickEvent(int x, int y);
	void HandleDragEvent(int x, int y);
	void HandleDragFromBoardView(int x, int y);
	void HandleDragFromUIView(int x, int y);
	void HandleZoomEvent(float dist, int origoX, int origoY);
	void HandleZoomEvent(float dist);
	void HandleZoomEndEvent();
    void HandleMultyDragEvent(int x0, int y0, int x1, int y1);

	void StartPlayerTurn(CPlayer* player, bool saveBoard = true);
	void StartComputerturn();
	bool EndComputerTurn();
	bool EndPlayerTurn(bool stillHaveTime = true);
	void NextPlayerTurn();
	void CurrentPlayerTurn(bool resumeGame);
	void HandlePlayerPass();
	void UndoLastStep();
	void UndoAllSteps();
	void UndoStep(size_t idx);
	void UndoStepAtPos(int x, int y);
	void RemovePlacedLetterSelAtPos(int x, int y);
	int GetPlayerStepIdxAtPos(int x, int y);
	void DealCurrPlayerLetters();
	void PlayerLetterReleased(size_t letterIdx);
	void GameLoop();
	void SetGameState(int state);
	void DealComputerLettersEvent();
	int PlayerWordHorizontal();
	EGameState GetGameState();
	CLetterModel* AddLetterToBoard(int x, int y, wchar_t c, float height);
	void AddWordSelectionAnimationForComputer();
	void UpdatePlayerScores();
	bool EndGameIfPlayerFinished();
	bool PlayerFinished();
	std::wstring GetNextPlayerName();
	int GetDifficulty();
	bool AllPlayersPassed();
	void SetPlayerLetters(size_t idx, const std::wstring& letters);
	std::wstring GetPlayerLetters(size_t idx, bool allLetters = false);
	bool GetPlayerProperties(size_t idx, std::wstring& name, int& score, glm::vec3& color);
	bool GameScreenActive();
	bool GameScreenActive(EGameState state);
	void ShowNextPlayerPopup();
	void ShowCurrPlayerPopup();
	bool TileAnimationFinished();
	bool PlayerLetterAnimationFinished();
	void StartDimmingAnimation(bool fadeIn);
	bool SelectionPosIllegal(int x, int y);
	void CheckAndUpdateTime(double& timeFromStart, double& timeFromPrev);
	bool IsGamePaused();
	void SetGamePaused(bool paused);
	void EndGame();
	void ExecuteTaskOnThread(const char* id, int threadId);
	void StopThreads();
	void AddNextPlayerTasksPass();
	bool AddNextPlayerTasksNormal(bool hasWordAnimation);

	glm::ivec2 GetUIElementSize(const wchar_t* id);
	float GetLetterSize();
	std::wstring GetTimeStr(int msec);
	std::string GetWorkingDir();
	void ShowLoadingScreen(bool show);
	std::wstring GetWordAtPos(bool horizontal, int& x, int& y);
	void SetBoardSize();
	void SetTileCounterCount();

	wchar_t GetChOnBoard(int x, int y) { return m_GameBoard(x, y).m_Char; }
	CWordTree::TNode* WordTreeRoot(wchar_t c) {return m_DataBase.GetWordTreeRoot(c);}
    TField& Board(int x, int y) {return m_GameBoard(x, y);}
    TField& TmpBoard(int x, int y) {return m_TmpGameBoard(x, y);}
	std::wstring CurrentPlayerName() {return m_CurrentPlayer ? m_CurrentPlayer->GetName() : L"";}
	int GetPlayerCount() {return m_Players.size();}
	void SetLastTouchOnBoardView(bool onBoardView) { m_LastTouchOnBoardView = onBoardView; }
	void SetLastTouchPos(int x, int y) { m_LastTouchX = x; m_LastTouchY = y; }
	CPlayer* GetCurrentPlayer() {return m_CurrentPlayer;}
	CPlayer* GetPlayer(size_t idx) {return m_Players[idx];}
	void SetCurentPlayer(size_t idx) {m_CurrentPlayer = m_Players[idx];}
 	void SaveState() { m_State->SaveGameState(); }
	void SaveTileAnims(std::ofstream& fileStream);
	void LoadTileAnims(std::ifstream& fileStream);
	void SaveCamera(std::ofstream& fileStream);
	void LoadCamera(std::ifstream& fileStream);
	void SaveWordAnims(std::ofstream& fileStream);
	void LoadWordAnims(std::ifstream& fileStream);
	size_t GetCurrentPlayerIdx();
	void LoadState();
	void LoadPlayerAndBoardState();
	void SetTaskFinished(const char* id) { m_TaskManager->SetTaskFinished(id); }
	void StopTaskThread() {m_TaskManager->StopThread();}
    void RevertGameBoard() {m_GameBoard = m_TmpGameBoard;}
    void HideLoadScreen()
    {
	    ShowLoadingScreen(false);
	    m_TaskManager->SetTaskFinished("hide_load_screen_task");
    }

    void ShowStartScreenTask();
    void ShowSavedScreenTask();
	void BeginGameTask();
	void ContinueGameTask();
	void InitRendererTask();
	void GenerateModelsTask();
	void InitGameScreenTask();
	void InitRankingsPanelTask();
	void ReturnToSavedStateTask();
    void InitPlayersTask();
    void StartGameLoopTask();
    void NextPlayerTask();
	void NextPlayerTaskOnThread();

    //TODO valamiert nem mukodik a perfect forwarding az osszes ilyen fuggvenynel, csak jobberteket lehet parameternek adni
	template <typename ClassType, typename... ArgTypes>
	std::shared_ptr<CTask> AddTask(ClassType* funcClass, typename CEvent<ClassType, ArgTypes...>::TFuncPtrType funcPtr, const char* id, CTask::ERunSource runThread, ArgTypes... args)
	{
		return m_TaskManager->AddTask(funcClass, funcPtr, id, runThread, std::forward<ArgTypes>(args)...);
	}


#ifdef PLATFORM_ANDROID
	void SetRendererObject(jobject obj)
	{
	    m_RendererObject = obj;
	}
#else
	std::queue<std::string> m_TaskToStartID;
	std::mutex m_TaskMutex;
#endif

	CRenderer* GetRenderer() { return m_Renderer; }
	CUIManager* GetUIManager() { return m_UIManager; }
	CTimerEventManager* GetTimerEventManager() {return m_TimerEventManager;}
	const std::vector<TPlayerStep>& GetPlayerSteps() {return m_PlayerSteps;}
	int GetLetterPoolCount() {return m_LetterPool.GetRemainingLetterCount(); }
	void AddPlacedLetterSelection(int x, int y) {m_PlacedLetterSelections.push_back(glm::ivec2(x, y));}
	void StartTask(const char* id) {m_TaskManager->StartTask(id);}
	void AddPlayerStep(wchar_t c, int letterIdx, int xPos, int yPos) {	m_PlayerSteps.emplace_back(c, xPos, yPos, letterIdx);}
	int GetLetterCount(int idx) {return m_LetterPool.GetLetterCount(idx);}
	void SetLetterCount(int idx, int count) {m_LetterPool.SetLetterCount(idx, count);}
	size_t GetCharacterCount() {return m_LetterPool.GetCharacterCount();}
	void PauseGameLoop(bool pause) {m_PauseGameLoop = pause;}
	bool ResumedOnGameScreen() {return GameScreenActive(m_SavedGameState);}


	void StartGameThread();
	bool GameStateFileFound();

#ifdef PLATFORM_ANDROID
	void RunTaskOnRenderThread(const char* id);
#endif

	glm::vec2 GetViewPosition(const char* viewId);
	bool PositionOnBoardView(int x, int y);
	
	bool HasEmptyFieldInWord(int& min, int& max);
	glm::ivec2 GetSelectionPosition();
	void SetDimmPanelOpacity(float opacity);

	void PauseGameEvent();
	void EndPlayerTurnEvent();
	void TopViewEvent();
	void GoToStartGameScrEvent();

	int frames = 0;
	long m_LastRenderTime;
	bool m_RenderTimeSet = false;
	long m_FrameTime = 0;

	//==============================================================================
	void UndoComp();
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
	CTaskManager* m_TaskManager;
	CRenderer* m_Renderer = nullptr;
	CGameThread* m_GameThread;


	CUIManager* m_UIManager = nullptr;
	CTimerEventManager* m_TimerEventManager = nullptr;
	CTileAnimationManager* m_TileAnimations = nullptr;
	CWordAnimationManager* m_WordAnimation = nullptr;
	CCameraAnimationManager* m_CameraAnimationManager = nullptr;
	CPlayerLetterAnimationManager* m_PlayerLetterAnimationManager = nullptr;
	CDimmBGAnimationManager* m_DimmBGAnimationManager = nullptr;
    CGameState* m_State;

	public: //TODO
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
	bool m_StopGameLoop = false;
	bool m_PauseGameLoop = false;

	bool m_StartOnGameScreen = false;

#ifdef PLATFORM_ANDROID
	JavaVM* m_JavaVM;
    jobject m_RendererObject = nullptr;
#endif

    EGameState m_GameState = EGameState::None;
    EGameState m_SavedGameState = EGameState::None;
	EGameState m_PrevGameState = EGameState::None;

	std::mutex m_GameStateLock;
	std::mutex m_GamePausedLock;
	std::mutex m_PlayerPopupLock;

	CPlayer* m_CurrentPlayer = nullptr;

	int m_ComputerWordIdx = -1;

	int m_LastTurnTimeChanged = 0;
	int m_TurnTimeStart = 0;

	bool m_GamePaused = false;
};

