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


class CRenderer;
class CUIManager;
class CTimerEventManager;
class CTileAnimationManager;
class CWordAnimationManager;
class CLetterModel;
class CCameraAnimationManager;

class CGameManager
{
public:
	
	enum EGameState { NextTurn, WaintingOnAnimation, WaitingForMessageBox, TurnInProgress, PlayerTurnEnd, ComputerTurnEnd, PlayerPass, ComputerPass, GameEnded, BeginGame, None };

	CGameManager();

	~CGameManager()
	{
		delete m_Renderer;
	}

	void AddPlayers(int playerCount, bool addComputer);
	void StartGame();
	void InitLetterPool();
	int CalculateScore(const TWordPos& word, std::vector<TWordPos>* crossingWords = nullptr);
	void InitRenderer(int surfaceWidth, int surfaceHeight);
	void InitUIManager();
	void RenderFrame();
	void RenderUI();
	void RenderTileAnimations();
	void AddWordSelectionAnimation(const std::vector<TWordPos>& wordPos, bool positive);

	void HandleReleaseEventFromBoardView(int x, int y);
	void HandleReleaseEventFromUIView(int x, int y);
	void HandleReleaseEvent(int x, int y);
	void HandleToucheEvent(int x, int y, bool onBoardView);
	void HandleDragEvent(int x, int y);
	void HandleDragFromBoardView(int x, int y);
	void HandleDragFromUIView(int x, int y) {}
	void HandleZoomEvent(float dist, int origoX, int origoY);
	void HandleZoomEvent(float dist);
	void HandleZoomEndEvent();
    void HandleMultyDragEvent(int x0, int y0, int x1, int y1);

	void StartPlayerTurn(CPlayer* player);
	void StartComputerturn();
	bool EndComputerTurn();
	bool EndPlayerTurn();
	void NextPlayerTurn();
	void UndoLastStep();
	void UndoAllSteps();
	void PlayerLetterClicked(unsigned letterIdx);
	void GameLoop();
	void SetGameState(int state);
	void DealComputerLetters();
	EGameState GetGameState();
	wchar_t GetCharOnBoard(int x, int y);
	CLetterModel* AddLetterToBoard(int x, int y, wchar_t c, float height);
	void AddWordSelectionAnimationForComputer();
	void UpdatePlayerScores();
	std::wstring GetNextPlayerName();
	wchar_t GetChOnBoard(int x, int y) { return m_GameBoard(x, y).m_Char; }

	std::wstring GetWordAtPos(bool horizontal, int& x, int& y);

	CWordTree::TNode* WordTreeRoot(wchar_t c) {return m_DataBase.GetWordTreeRoot(c);}
	TField& Board(int x, int y) {return m_GameBoard(x, y);}
	std::wstring CurrentPlayerName() {return m_CurrentPlayer ? m_CurrentPlayer->GetName() : L"";}

	CRenderer* GetRenderer() {return m_Renderer;}
	glm::vec2 GetViewPosition(const char* viewId);


	void EndPlayerTurnEvent();
	void BackSpaceEvent();
	void TopViewEvent();

	int frames = 0;
	std::chrono::high_resolution_clock::time_point LastRenderTime;
	//==============================================================================
	#ifndef PLATFORM_ANDROID
	HWND m_HWND;
	#endif
	bool m_GameEnded = false;
	std::wstring GetScoreString()
	{
//		int LetterCount = m_LetterPool.GetLetterCount();
		std::wstringstream ss;
//		ss << "Jatekos : " << m_Players[0]->GetScore() << "      Computer : " << m_Computer->GetScore() << "       Betuk  : " << LetterCount << "   *********** " << m_CurrentPlayer->GetName() << " ***********";
		return ss.str();
	}

	std::wstring GetWinnerString()
	{
		std::wstringstream ss;
		ss << "A Gyoztes " << (m_Players[0]->GetScore() > m_Players[1]->GetScore() ? m_Players[0]->GetName() : m_Players[1]->GetName());
		return ss.str();
	}


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

	public: //TODO
	bool m_Dragged = false;
	bool m_LastTouchOnBoardView;
	int m_LastTouchX;
	int m_LastTouchY;
	int m_TouchX = -1;
	int m_TouchY;
	int m_SurfaceWidth; //TODO atnevezni surfaceheight ...
	int m_SurfaceHeigh;

	EGameState m_GameState = EGameState::None;
	std::mutex m_GameStateLock;

	CPlayer* m_CurrentPlayer = nullptr;

	int m_FirstPlayerLetterX = -1;
	int m_FirstPlayerLetterY = -1;
	int m_SecondPlayerLetterX = -1;
	int m_SecondPlayerLetterY = -1;

	int m_ComputerWordIdx = -1;
};

