#include "stdafx.h"
#include "GameManager.h"
#include "Renderer.h"
#include "UIManager.h"
#include "TileAnimationManager.h"
#include "Config.h"
#include "GameBoard.h"
#include "UIButton.h"
#include "UIPlayerLetters.h"
#include "UIMessageBox.h"
#include "GridLayout.h"
#include "opengl.h"
#include "TimerEventManager.h"
#include "TileAnimationManager.h"
#include "WordAnimationManager.h"
#include "CameraAnimationManager.h"
#include "PlayerLetterAnimationManager.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>


//TODO computer jateknal preferalja a palya kozepe fele levo szvakat azonos pontszam eseten!!

CGameManager::CGameManager()
{
	CConfig::LoadConfigs("config.txt");

	m_DataBase.LoadDataBase("dic.txt");

	m_TimerEventManager = new CTimerEventManager();
	m_TileAnimations = new CTileAnimationManager(m_TimerEventManager, this);
	m_WordAnimation = new CWordAnimationManager(m_TimerEventManager, this); //TODO!!!!!!!!!!!
	m_CameraAnimationManager = new CCameraAnimationManager(m_TimerEventManager, this); //TODO!!!!!!!!!!!
	m_PlayerLetterAnimationManager = new CPlayerLetterAnimationManager(this, m_TimerEventManager); //TODO!!!!!!!!!!!
}


void CGameManager::AddPlayers(int playerCount, bool addComputer)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	for (int i = 0; i < playerCount; ++i)
	{
		m_Players.push_back(new CPlayer(this));
		m_LetterPool.DealLetters(m_Players.back()->GetLetters());
		m_UIManager->AddPlayerLetters(m_Players.back(), m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorGridData8x4());
		m_UIManager->PositionPlayerLetters(m_Players.back()->GetName().c_str());
		m_UIManager->GetPlayerLetters(m_Players.back()->GetName().c_str())->ShowLetters(false);
	}

	if (addComputer)
	{ 
		m_Computer = new CComputer(this);
		m_LetterPool.DealLetters(m_Computer->GetLetters());
		m_Players.push_back(m_Computer);
		//TODO ezeket a uimanageres fuggvenyeket osszevonni egybe a uimanagerben
		m_UIManager->AddPlayerLetters(m_Players.back(), m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorGridData8x4());
		m_UIManager->PositionPlayerLetters(m_Players.back()->GetName().c_str());
		m_UIManager->GetPlayerLetters(m_Players.back()->GetName().c_str())->ShowLetters(false);
	}

	m_UIManager->SetTileCounterValue(m_LetterPool.GetRemainingLetterCount());
}

void CGameManager::InitLetterPool()
{
	m_LetterPool.Init();
}

void CGameManager::FinishRenderInit()
{
#ifdef PLATFORM_ANDROID
	JNIEnv *env;
	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);
	jclass Class = env->FindClass("com/example/szocsata_android/OpenGLRenderer");
	jmethodID Method = env->GetMethodID(Class, "FinishRenderInit", "()V");
	env->CallVoidMethod(m_RendererObject, Method);
#else
	SetTileCount();
	InitBasedOnTileCount();
	EndInitRenderer();
	InitPlayers();
	SetGameState(CGameManager::BeginGame);
#endif
}

void CGameManager::SetTileCount()
{
	int TileCount = m_UIManager->GetBoardSize();

	if (TileCount == 0)
		TileCount = 7;
	else if (TileCount == 1)
		TileCount = 8;
	else if (TileCount == 2)
		TileCount = 9;
	else if (TileCount == 3)
		TileCount = 10;

	CConfig::AddConfig("tile_count", TileCount);
}


void CGameManager::InitBasedOnTileCount()
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	m_GameBoard.SetSize(TileCount);
	m_TmpGameBoard.SetSize(TileCount);
	CompGameBoard.SetSize(TileCount);

	InitLetterPool();
	PositionUIElements();
	m_UIManager->ActivateStartScreen(false);
}

void CGameManager::InitPlayers()
{
	int PlayerCount = m_UIManager->GetPlayerCount() + 1;
	AddPlayers(PlayerCount, m_UIManager->ComputerOpponentEnabled());
	m_UIManager->InitScorePanel();
	UpdatePlayerScores();
}

void CGameManager::StartGame()
{
	ShowNextPlayerPopup();
}

void CGameManager::AddWordSelectionAnimation(const std::vector<TWordPos>& wordPos, bool positive)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	for (size_t j = 0; j < wordPos.size(); ++j)
	{
		int x = wordPos[j].m_X;
		int y = TileCount - wordPos[j].m_Y - 1;

		for (size_t i = 0; i < wordPos[j].m_WordLength; ++i)
		{
			m_TileAnimations->AddTile(x, y);
			x += wordPos[j].m_Horizontal ? 1 : 0;
			y -= wordPos[j].m_Horizontal ? 0 : 1;
		}
	}

	m_TileAnimations->StartAnimation(positive);
}


void CGameManager::StartPlayerTurn(CPlayer* player)
{
	m_CurrentPlayer = player;
	SetGameState(EGameState::TurnInProgress);
	m_TmpGameBoard = m_GameBoard;
}

bool CGameManager::TileAnimationFinished() 
{ 
	return m_TileAnimations->Finished(); 
}

std::wstring CGameManager::GetTimeStr(int msec)
{
	int Minutes = msec / 60000;
	int Seconds = (msec % 60000) / 1000;

	std::wstringstream ss;

	if (Minutes)
		ss << Minutes << ":";
	
	if (Seconds < 10)
		ss << "0";

	ss << Seconds;

	return ss.str().c_str();
}

void CGameManager::CheckAndUpdateTime(double& timeFromStart, double& timeFromPrev)
{
	int TimeLimit = m_UIManager->GetTimeLimit();

	//update time
	if (m_LastTurnTimeChanged >= 1000 || m_LastTurnTimeChanged == 0)
	{
		m_LastTurnTimeChanged = 0;
		std::wstring RemainingTimeStr = GetTimeStr(TimeLimit - timeFromStart);
		m_UIManager->SetRemainingTimeStr(RemainingTimeStr.c_str());
	}
	else
		m_LastTurnTimeChanged += timeFromPrev;


	//lejart az ido uj jatekos jon
	if (timeFromStart >= TimeLimit)
	{
		m_TimerEventManager->StopTimer("time_limit_event");

		//ha jatekos kore volt, megnezzuk hogy a lerakott betuk ervenyesek e
		if (m_CurrentPlayer->GetName() != L"computer" && !EndPlayerTurn(false))
			UndoAllSteps();
	
		m_UIManager->SetDraggedPlayerLetter(false, 0, glm::vec2(0.f, 0.f), glm::vec2(0.f, 0.f), true);
		m_Renderer->DisableSelection();
	}
}

bool CGameManager::SelectionPosIllegal(int x, int y)
{
	//nem a tablara tettuk a betut
	if (x == -1 || y == -1)
		return true;

	//ha csak egy betu van leteve, es a selection nem abba a sorba es oszlopba van
	if (m_PlayerSteps.size() == 1 && x != m_PlayerSteps[0].m_XPosition && y != m_PlayerSteps[0].m_YPosition)
		return true;

	//ha mar ketto vagy tobb betu van leteve, es a selection nem abba a sorba es oszlopba van
	if (m_PlayerSteps.size() >= 2 && !(m_PlayerSteps[0].m_XPosition == m_PlayerSteps[1].m_XPosition && x == m_PlayerSteps[0].m_XPosition || m_PlayerSteps[0].m_YPosition == m_PlayerSteps[1].m_YPosition && y == m_PlayerSteps[0].m_YPosition))
		return true;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	//ha mar 5 elemes a torony amire rakni akarunk	
	if (m_GameBoard(x, TileCount - y - 1).m_Height == 5)
		return true;
	
	//ha mar egy elhelyezett beture akarubk rakni
	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (m_PlayerSteps[i].m_XPosition == x && m_PlayerSteps[i].m_YPosition == y)
			return true;
	}

	return false;
}

bool CGameManager::PlayerLetterAnimationFinished()
{ 
	return m_PlayerLetterAnimationManager->Finished();
}

void CGameManager::ShowNextPlayerPopup()
{
	const std::lock_guard<std::mutex> lock(m_PlayerPopupLock);

	bool ShowPopup = !m_NextPlayerPopupShown && m_PlayerLetterAnimationManager->Finished() && m_TileAnimations->Finished();

	if (ShowPopup)
	{
		m_NextPlayerPopupShown = true;
		m_UIManager->EnableGameButtons(true);
		m_UIManager->SetRemainingTimeStr(GetTimeStr(m_UIManager->GetTimeLimit()).c_str());
		m_UIManager->ShowMessageBox(CUIMessageBox::Ok, GetNextPlayerName().c_str());
	}
}


bool CGameManager::GameScreenActive()
{
	return (GetGameState() != EGameState::OnRankingsScreen && GetGameState() != EGameState::OnStartGameScreen && GetGameState() != EGameState::OnStartScreen);
}


bool CGameManager::GetPlayerNameScore(size_t idx, std::wstring& name, int& score)
{
	if (m_Players.size() <= idx)
		return false;

	name = m_Players[idx]->GetName();
	score = m_Players[idx]->GetScore();

	return true;
}

bool CGameManager::AllPlayersPassed()
{
	bool GameEnded = true;

	for (size_t i = 0; i < m_Players.size(); ++i)
		GameEnded &= m_Players[i]->m_Passed;

	return GameEnded;
}

int CGameManager::GetDifficulty() 
{ 
	return m_UIManager->GetDifficulty(); 
}

std::wstring CGameManager::GetNextPlayerName()
{
	int NextPlayerIdx;

	if (m_CurrentPlayer)
	{
		int CurrPlayerIdx = -1;
		while (m_Players[++CurrPlayerIdx] != m_CurrentPlayer);
		NextPlayerIdx = CurrPlayerIdx == m_Players.size() - 1 ? 0 : CurrPlayerIdx + 1;
	}
	else
		NextPlayerIdx = 0;
	
	return m_Players[NextPlayerIdx]->GetName();
}


void CGameManager::NextPlayerTurn()
{
	int NextPlayerIdx;

	if (!m_CurrentPlayer)
	{
		NextPlayerIdx = 0;
		m_CurrentPlayer = m_Players[0];
	}
	else
	{ 
		m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetVisible(false);
		int CurrPlayerIdx = -1;
		while (m_Players[++CurrPlayerIdx] != m_CurrentPlayer);
		NextPlayerIdx = CurrPlayerIdx == m_Players.size() - 1 ? 0 : CurrPlayerIdx + 1;
	}

	//letette e az osszes betujet a jatekos
	if (m_CurrentPlayer->GetLetterCount() == 0)
	{
		m_UIManager->InitRankingsPanel();
		SetGameState(EGameState::GameEnded);
		return;
	}

	m_Players[NextPlayerIdx]->m_Passed = false;
	SetGameState(EGameState::TurnInProgress);

	if (m_UIManager->GetTimeLimit() != -1)
	{ 
		m_TimerEventManager->AddTimerEvent(this, &CGameManager::CheckAndUpdateTime, nullptr, "time_limit_event");
		m_TimerEventManager->StartTimer("time_limit_event");
		m_LastTurnTimeChanged = 0;
	}

	m_CurrentPlayer = m_Players[NextPlayerIdx];

	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility(CBinaryBoolList());
	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetVisible(true);

	if (m_Players[NextPlayerIdx]->GetName() == L"computer")
	{
		m_UIManager->EnableGameButtons(false);
		StartComputerturn();
	}
	else
	{
		m_UIManager->EnableGameButtons(true);
		StartPlayerTurn(m_Players[NextPlayerIdx]);
	}
}

void CGameManager::EndGameAfterLastPass()
{
	m_UIManager->InitRankingsPanel();
	SetGameState(EGameState::GameEnded);
}


void CGameManager::HandlePlayerPass()
{
	m_CurrentPlayer->m_Passed = true;
	bool AllPassed = AllPlayersPassed();
	m_PlacedLetterSelections.clear();
	m_PlayerSteps.clear();

	//TODO biztos passzolni akarsz msgbox!
	m_UIManager->ShowToast(L"passz", AllPassed);
}

//ret 1 - horizontal 0 - not horizontal -1 - undefined
int CGameManager::PlayerWordHorizontal()
{
	if (m_PlayerSteps.size() < 2)
		return -1;

	if (m_PlayerSteps[0].m_YPosition == m_PlayerSteps[1].m_YPosition)
		return 1;

	return 0;
}

bool CGameManager::EndPlayerTurn(bool stillHaveTime)
{	
	m_NextPlayerPopupShown = false;
	m_Renderer->HideSelection(true);

	//jatekos passz
	if (m_CurrentPlayer->GetUsedLetterCount() == 0)
	{
		HandlePlayerPass();
		return false;
	}

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	int Horizontal = PlayerWordHorizontal();
	int BoardX = m_PlayerSteps[0].m_XPosition;
	int BoardY = TileCount - m_PlayerSteps[0].m_YPosition - 1;

	std::wstring PlayerWord;

	//ha mar 2 betut letettunk tudjuk a szo orientaciojat
	if (Horizontal != -1)
		PlayerWord = GetWordAtPos(Horizontal, BoardX, BoardY);

	//ha csak 1 betut tettunk le leteszteljuk vizszintesen v fuggolegesen ertelmes e a szo
	else
	{
		PlayerWord = GetWordAtPos(true, BoardX, BoardY);

		if (PlayerWord.length() == 0)
		{
			PlayerWord = GetWordAtPos(false, BoardX, BoardY);
			Horizontal = false;
		}
		else
			Horizontal = true;
	}

	std::vector<TWordPos> CrossingWords;
	TWordPos WordPos(&PlayerWord, BoardX, BoardY, Horizontal);


	//ha az elhelyezett szoban ures mezok vannak
	int WordStart, WordEnd;

	if (HasEmptyFieldInWord(WordStart, WordEnd))
	{
		WordPos.m_WordLength = WordEnd - WordStart + 1;
		WordPos.m_X = WordPos.m_Horizontal ? WordStart : WordPos.m_X;
		WordPos.m_Y = !WordPos.m_Horizontal ? TileCount - WordEnd - 1 : WordPos.m_Y;

		//ha meg nem telt le az ido jeloljuk ki pirossal a hibas crossingword szot
		if (stillHaveTime)
		{
			CrossingWords.push_back(WordPos);
			AddWordSelectionAnimation(CrossingWords, false);
		}
		//ha letelt az ido jatekos passz
		else
			HandlePlayerPass();

		return false;
	}

	//ha nem letezik a letett szo
	if (PlayerWord.empty() || !m_DataBase.WordExists(PlayerWord)) //TODO miert lehet PlayerWord.empty()!!!!
	{ 
		//ha meg nem telt le az ido jeloljuk ki pirossal a hibas crossingword szot
		if (stillHaveTime)
		{
			CrossingWords.push_back(WordPos);
			AddWordSelectionAnimation(CrossingWords, false);
		}
		//ha letelt az ido jatekos passz
		else
			HandlePlayerPass();

		return false;
	}

	//ha keresztezo szavakat elrontottunk az elhelyezett szoval
	CrossingWords.reserve(PlayerWord.length());
	int Score = CalculateScore(WordPos, &CrossingWords);

	if (Score == 0)
	{
		//ha meg nem telt le az ido jeloljuk ki pirossal a hibas szot
		if (stillHaveTime)
			AddWordSelectionAnimation(CrossingWords, false);
		//ha letelt az ido jatekos passz
		else
			HandlePlayerPass();

		return false;
	}

	DealCurrPlayerLetters();
	AddWordSelectionAnimation(CrossingWords, true);
	m_PlacedLetterSelections.clear();
	m_PlayerSteps.clear();

	m_TimerEventManager->StopTimer("time_limit_event");

	m_CurrentPlayer->AddScore(Score);
	UpdatePlayerScores();
	SetGameState(EGameState::WaintingOnAnimation);

	return true;
}

void CGameManager::DealCurrPlayerLetters()
{
	bool LetterPoolEmpty = m_LetterPool.GetRemainingLetterCount() != 0;
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
	m_LetterPool.DealLetters(m_CurrentPlayer->GetLetters());
	m_CurrentPlayer->GetLetters().erase(remove(m_CurrentPlayer->GetLetters().begin(), m_CurrentPlayer->GetLetters().end(), ' '), m_CurrentPlayer->GetLetters().end());
	PlayerLetters->SetLetters();

	m_UIManager->PositionPlayerLetters(m_CurrentPlayer->GetName().c_str());
	PlayerLetters->OrderLetterElements();

	if (LetterPoolEmpty)
	{
		glm::vec2 TileCounterPos = m_UIManager->GetTileCounterPos();

		for (size_t i = 0; i < m_CurrentPlayer->GetLetters().length(); ++i)
		{
			if (m_CurrentPlayer->LetterUsed(i))
			{
				PlayerLetters->GetChild(i)->Scale(0.f);
				m_PlayerLetterAnimationManager->AddAnimation(PlayerLetters->GetChild(i), PlayerLetters->GetChild(i)->GetWidth(), TileCounterPos.x, TileCounterPos.y, PlayerLetters->GetChild(i)->GetXPosition(), PlayerLetters->GetChild(i)->GetYPosition());
			}
		}
		m_PlayerLetterAnimationManager->StartAnimations();
	}

	PlayerLetters->SetLetterVisibility(CBinaryBoolList());
	m_CurrentPlayer->ResetUsedLetters();

}

void CGameManager::DealComputerLettersEvent()
{
	DealCurrPlayerLetters();
}

bool CGameManager::EndComputerTurn()
{
	m_NextPlayerPopupShown = false;
	m_TimerEventManager->StopTimer("time_limit_event");

	int TileCount;

	CConfig::GetConfig("tile_count", TileCount);

	TComputerStep ComputerStep;
	TWordPos ComputerWord;
	std::vector<TWordPos>* CrossingWords = nullptr;
	bool ComputerPass = (m_Computer->BestWordCount() <= m_ComputerWordIdx);

	if (!ComputerPass)
	{
		ComputerStep = m_Computer->BestWord(m_ComputerWordIdx);
		ComputerWord = ComputerStep.m_Word;
		CrossingWords = &ComputerStep.m_CrossingWords;
		m_Computer->ResetUsedLetters();
	}

	//computer passz
	if (ComputerPass || !ComputerWord.m_Word)
	{
		HandlePlayerPass();
		return false;
	}

	bool LetterAdded = false;
	int ComputerStepDelay;

	CConfig::GetConfig("computer_step_delay", ComputerStepDelay);

	size_t WordLength = ComputerWord.m_Word->length();

	std::vector<size_t> LetterIndices = m_CurrentPlayer->GetLetterIndicesForWord(*ComputerWord.m_Word);
	m_WordAnimation->AddWordAnimation(*ComputerWord.m_Word, LetterIndices, m_UIManager->GetPlayerLetters(m_Computer->GetName()), ComputerWord.m_X, TileCount - ComputerWord.m_Y - 1, ComputerWord.m_Horizontal);
	m_GameBoard.AddWord(ComputerWord);
	UpdatePlayerScores();
	SetGameState(EGameState::WaintingOnAnimation);


	return false;
}

void CGameManager::StartComputerturn()
{
	//	CompGameBoard = m_GameBoard;
	//	CompLetters = m_Computer->m_Letters;	
	m_TmpGameBoard = m_GameBoard;
	m_Computer->CalculateStep();

	int GameDifficulty;
	int BestWordCount = m_Computer->BestWordCount();

	//computer passz, egy szot sem talalt ami megfelelt a kriteriumoknak
	if (m_Computer->BestWordCount() == 0)
	{
		HandlePlayerPass();
		return;
	}

	CConfig::GetConfig("game_difficulty", GameDifficulty);

	int WordCount;

	m_ComputerWordIdx = std::rand() / ((RAND_MAX + 1u) / m_Computer->BestWordCount());

	if (m_UIManager->GetDifficulty() == 3)
		m_ComputerWordIdx = 0;

	if (m_Computer->BestWordCount() && m_ComputerWordIdx < m_Computer->BestWordCount())
	{
		TComputerStep ComputerStep = m_Computer->BestWord(m_ComputerWordIdx);

		m_Computer->AddScore(ComputerStep.m_Score);
		m_Computer->SetUsedLetters(ComputerStep.m_UsedLetters);
	}
	else
	{
		//coputer passz
		HandlePlayerPass();
		return;
	}

	EndComputerTurn();
}

void CGameManager::UpdatePlayerScores()
{
	m_UIManager->UpdateScorePanel();
}

CLetterModel* CGameManager::AddLetterToBoard(int x, int y, wchar_t c, float height)
{
	return m_Renderer->AddLetterToBoard(x, y, c, height, true);
}

void CGameManager::AddWordSelectionAnimationForComputer()
{
	AddWordSelectionAnimation(m_Computer->BestWord(m_ComputerWordIdx).m_CrossingWords, true);
}

void CGameManager::SetGameState(int state)
{
	const std::lock_guard<std::mutex> lock(m_GameStateLock);
	m_GameState = static_cast<EGameState>(state);
}

CGameManager::EGameState CGameManager::GetGameState()
{
	const std::lock_guard<std::mutex> lock(m_GameStateLock);
	return m_GameState;
}

void CGameManager::GameLoop()
{
	if (GetGameState() == EGameState::BeginGame)
		StartGame();

	if (GetGameState() != EGameState::GameEnded)
	{
		if (GetGameState() == EGameState::WaitingForMessageBox && !CUIMessageBox::m_ActiveMessageBox)
			SetGameState(EGameState::NextTurn);

		if (GetGameState() == EGameState::NextTurn)
			NextPlayerTurn();

		m_TimerEventManager->Loop();
	}
	else
	{ 
		m_Renderer->ClearBuffers();
		SetGameState(EGameState::OnRankingsScreen);
	}
}


std::wstring CGameManager::GetWordAtPos(bool horizontal, int& x, int& y)
{
	std::wstring res;
	res += m_GameBoard(x, y).m_Char;
	int lx = x;
	int ly = y;

	if (!horizontal)
	{
		for (int i = y - 1; i >= 0; --i)
		{
			if (m_GameBoard(x, i).m_Char == L'*')
				break;
			y--;
			res = m_GameBoard(x, i).m_Char + res;
		}

		for (int i = ly + 1; i < m_GameBoard.Size(); ++i)
		{
			if (m_GameBoard(x, i).m_Char == L'*')
				break;

			res += m_GameBoard(x, i).m_Char;
		}
	}
	else
	{
		for (int i = x - 1; i >= 0; --i)
		{
			if (m_GameBoard(i, y).m_Char == L'*')
				break;

			x--;
			res = m_GameBoard(i, y).m_Char + res;
		}

		for (int i = lx + 1; i < m_GameBoard.Size(); ++i)
		{
			if (m_GameBoard(i, y).m_Char == L'*')
				break;

			res += m_GameBoard(i, y).m_Char;
		}
	}

	return res.length() == 1 ? std::wstring() : res;
}

int CGameManager::CalculateScore(const TWordPos& word, std::vector<TWordPos>* crossingWords)
{
	bool CrossingWordsValid = true;
	int Score = 0;
	CGameBoard& board = (m_CurrentPlayer == m_Computer ? m_GameBoard : m_TmpGameBoard);
	bool SingleHeightFound;

	if (word.m_Horizontal)
	{
		SingleHeightFound = false;

		for (int i = word.m_X; i < word.m_X + word.m_Word->length(); ++i)
		{ 
			int LetterScore = (board(i, word.m_Y).m_Char != word.m_Word->at(i - word.m_X) ? 1 : 0);
			Score += LetterScore + board(i, word.m_Y).m_Height;

			if (board(i, word.m_Y).m_Height + LetterScore == 1)
				SingleHeightFound = true;
		}

        (*crossingWords).emplace_back(nullptr, word.m_X, word.m_Y, true, word.m_Word->length());

		if (!SingleHeightFound)
            return 0;

		for (int i = word.m_X; i < word.m_X + word.m_Word->length(); ++i)
		{
			int x = i;
			int y = word.m_Y;

			if (board(x, y).m_Char == word.m_Word->at(i - word.m_X))
				continue;

			std::wstring CrossingWord = GetWordAtPos(false, x, y);

			if (!CrossingWord.empty())
			{
				CrossingWord.at(word.m_Y - y) = word.m_Word->at(i - word.m_X);

				if (!m_DataBase.WordExists(CrossingWord))
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());
					CrossingWordsValid = false;
					break;
				}

				SingleHeightFound = false;

				for (int i = y; i < y + CrossingWord.length(); ++i)
				{ 
					int LetterScore = i == word.m_Y ? 1 : 0;
					Score += board(x, i).m_Height;

					if (board(x, i).m_Height + LetterScore == 1)
						SingleHeightFound = true;
				}

				if (!SingleHeightFound)
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());
                    return 0;
                }

				Score++;
				(*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());
			}
		}
	}
	else
	{
		SingleHeightFound = false;

		for (int i = word.m_Y; i < word.m_Y + word.m_Word->length(); ++i)
		{ 
			int LetterScore = (board(word.m_X, i).m_Char != word.m_Word->at(i - word.m_Y) ? 1 : 0);
			Score += LetterScore + board(word.m_X, i).m_Height;

			if (board(word.m_X, i).m_Height + LetterScore == 1)
				SingleHeightFound = true;
		}

        (*crossingWords).emplace_back(nullptr, word.m_X, word.m_Y, false, word.m_Word->length());

		if (!SingleHeightFound)
            return 0;

		for (int i = word.m_Y; i < word.m_Y + word.m_Word->length(); ++i)
		{
			int x = word.m_X;
			int y = i;

			if (board(x, y).m_Char == word.m_Word->at(i - word.m_Y))
				continue;

			std::wstring CrossingWord = GetWordAtPos(true, x, y);

			if (!CrossingWord.empty())
			{
				CrossingWord.at(word.m_X - x) = word.m_Word->at(i - word.m_Y);

				if (!m_DataBase.WordExists(CrossingWord))
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());
					CrossingWordsValid = false;
					break;
				}

				SingleHeightFound = false;

				for (int i = x; i < x + CrossingWord.length(); ++i)
				{
					int LetterScore = i == word.m_X ? 1 : 0;

					Score += board(i, y).m_Height;

					if (board(i, y).m_Height + LetterScore == 1)
						SingleHeightFound = true;
				}

				if (!SingleHeightFound)
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());
                    return 0;
                }

				Score++;
				(*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());
			}
		}
	}

	return CrossingWordsValid ? Score : 0;
}

void CGameManager::PlayerLetterClicked(unsigned letterIdx)
{
	int SelX, SelY;
	m_Renderer->GetSelectionPos(SelX, SelY);

	if (SelX == -1 || SelY == -1)
		return;

	if (m_PlayerSteps.size() == 1 && SelX != m_PlayerSteps[0].m_XPosition && SelY != m_PlayerSteps[0].m_YPosition)
		return;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	if (m_GameBoard(SelX, TileCount - SelY - 1).m_Height == 5 || m_GameBoard(SelX, TileCount - SelY - 1).m_Char == m_CurrentPlayer->GetLetters()[letterIdx])
		return;

	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (m_PlayerSteps[i].m_XPosition == SelX && m_PlayerSteps[i].m_YPosition == SelY)
			return;
	}

	wchar_t PlacedLetter = m_CurrentPlayer->GetLetters()[letterIdx];
	m_WordAnimation->AddWordAnimation(std::wstring(1, PlacedLetter), std::vector<size_t>{letterIdx}, m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName()), SelX, SelY, true, false);
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());

	m_PlayerSteps.emplace_back(PlacedLetter, SelX, SelY, letterIdx);

	int Horizontal = PlayerWordHorizontal();

	if (Horizontal == 1 && SelY != m_PlayerSteps[0].m_YPosition || Horizontal == 0 && SelX != m_PlayerSteps[0].m_XPosition)
		return;


	int BoardY = TileCount - SelY - 1;
	m_GameBoard(SelX, BoardY).m_Char = PlacedLetter;
	m_GameBoard(SelX, BoardY).m_Height++;

	if ((Horizontal == 1 && SelX < TileCount - 1 || Horizontal == 0 && SelY > 0))
	{
		SelX += Horizontal ? 1 : 0;
		SelY -= Horizontal ? 0 : 1;
	
		if (!SelectionPosIllegal(SelX, SelY))
		{
			m_Renderer->HideSelection(false);
			m_Renderer->SelectField(SelX, SelY);
		}
		else
			m_Renderer->HideSelection(true);
	}
	else
		m_Renderer->HideSelection(true);
}

void CGameManager::PositionUIElements()
{
	m_UIManager->PositionUIElements();
}

void CGameManager::InitUIManager()
{
	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);
	ShowFps &= ConfigFound;

	m_UIManager = new CUIManager(this, m_TimerEventManager);
	m_UIManager->InitUIElements(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData8x8(), m_Renderer->GetSquareColorGridData8x4());

	if (ShowFps)
		m_UIManager->SetText(L"ui_fps_text", L"fps : 0");

	m_TileAnimations->SetUIManager(m_UIManager);
}

void CGameManager::StartInitRenderer(int surfaceWidth, int surfaceHeight)
{
	m_SurfaceWidth = surfaceWidth;
	m_SurfaceHeigh = surfaceHeight;

	CConfig::AddConfig("window_width", surfaceWidth);
	CConfig::AddConfig("window_height", surfaceHeight);

	m_Renderer = new CRenderer(surfaceWidth, surfaceHeight, this);
	m_Renderer->StartInit();
}


void CGameManager::EndInitRenderer()
{
	m_Renderer->EndInit();
}

glm::vec2 CGameManager::GetViewPosition(const char* viewId) 
{ 
	return m_Renderer->GetViewPosition(viewId); 
}

void CGameManager::HandleReleaseEvent(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);
	
    m_TouchX = -1;

	if (m_LastTouchOnBoardView)
		HandleReleaseEventFromBoardView(x, WindowHeigth - y);
	else
		HandleReleaseEventFromUIView(x, WindowHeigth - y);
}

void CGameManager::HandleReleaseEventFromBoardView(int x, int y)
{
	if (!GameScreenActive())
		return;

	m_Renderer->CalculateScreenSpaceGrid();
	m_Dragged = false;

	float Offset = std::sqrtf((m_LastTouchX - x) * (m_LastTouchX - x) + (m_LastTouchY - y) * (m_LastTouchY - y));

	if (Offset < 3) //TODO configbol
	{
		m_PlacedLetterTouchX = -1;
		TPosition p = m_Renderer->GetTilePos(m_LastTouchX, m_LastTouchY);

		if (p.x != -1)
			m_Renderer->SelectField(p.x, p.y);
	}
}

void CGameManager::HandleReleaseEventFromUIView(int x, int y)
{
	m_Dragged = false;
	m_UIManager->HandleReleaseEvent(x, y);
}

void CGameManager::UndoAllSteps()
{
	while (m_PlayerSteps.size())
		UndoLastStep();
}

void CGameManager::UndoLastStep()
{
	UndoStep(m_PlayerSteps.size() - 1);
}

void CGameManager::UndoStepAtPos(int x, int y)
{
	int Idx;

	if ((Idx = GetPlayerStepIdxAtPos(x, y)) != -1)
		UndoStep(Idx);
}

void CGameManager::RemovePlacedLetterSelAtPos(int x, int y)
{
	for (size_t i = 0; i < m_PlacedLetterSelections.size(); ++i)
	{
		if (m_PlacedLetterSelections[i].x == x && m_PlacedLetterSelections[i].y == y)
		{
			m_PlacedLetterSelections[i] = m_PlacedLetterSelections.back();
			m_PlacedLetterSelections.pop_back();
			return;
		}
	}
}

int CGameManager::GetPlayerStepIdxAtPos(int x, int y)
{
	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
		if (m_PlayerSteps[i].m_XPosition == x && m_PlayerSteps[i].m_YPosition == y)
			return i;

	return -1;	
}

void CGameManager::UndoStep(size_t idx)
{
	if (m_PlayerSteps.size() <= idx)
		return;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	int BoardY = TileCount - m_PlayerSteps[idx].m_YPosition - 1;
	int BoardX = m_PlayerSteps[idx].m_XPosition;

	m_GameBoard(BoardX, BoardY).m_Char = m_TmpGameBoard(BoardX, BoardY).m_Char;
	m_GameBoard(BoardX, BoardY).m_Height--;

	m_CurrentPlayer->SetLetter(m_PlayerSteps[idx].m_LetterIdx, m_PlayerSteps[idx].m_Char);
	m_CurrentPlayer->SetLetterUsed(m_PlayerSteps[idx].m_LetterIdx, false);

	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility(m_CurrentPlayer->GetUsedLetters());

	m_Renderer->RemoveTopLetter(BoardX, m_PlayerSteps[idx].m_YPosition);
	m_PlayerSteps[idx] = m_PlayerSteps.back();
	m_PlayerSteps.pop_back();

}

//a lerakott betuk kozott maradt e ures mezo
bool CGameManager::HasEmptyFieldInWord(int& min, int& max)
{
	min = 10;
	max = 0;

	int Horizontal = PlayerWordHorizontal();

	if (Horizontal == -1)
		return false;

	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (Horizontal == 1 && m_PlayerSteps[i].m_XPosition > max || Horizontal == 0 && m_PlayerSteps[i].m_YPosition > max)
			max = Horizontal ? m_PlayerSteps[i].m_XPosition : m_PlayerSteps[i].m_YPosition;

		if (Horizontal == 1 && m_PlayerSteps[i].m_XPosition < min || Horizontal == 0 && m_PlayerSteps[i].m_YPosition < min)
			min = Horizontal ? m_PlayerSteps[i].m_XPosition : m_PlayerSteps[i].m_YPosition;
	}

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	for (int i = min; i <= max; ++i)
	{
		if (Horizontal == 1 && m_GameBoard(i, TileCount - m_PlayerSteps[0].m_YPosition - 1).m_Height == 0)
			return true;
		else if (Horizontal == 0 && m_GameBoard(m_PlayerSteps[0].m_XPosition, TileCount - i - 1).m_Height == 0)
			return true;
	}

	return false;
}


//ha a lerakott szo utolso karaktere utan, ervenyes mezo van a tablan, amit ki lehet valasztani
//akkor adjuk vissza a koordinatait
glm::ivec2 CGameManager::GetSelectionPosition()
{
	int Horizontal = PlayerWordHorizontal();

	if (Horizontal == -1)
		return glm::ivec2(-1, -1);

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);
	int Lim = -1;
	size_t Idx;

	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (Horizontal == 1 && m_PlayerSteps[i].m_XPosition > Lim || Horizontal == 0 && (m_PlayerSteps[i].m_YPosition < Lim || Lim == -1))
		{
			Lim = Horizontal ? m_PlayerSteps[i].m_XPosition : m_PlayerSteps[i].m_YPosition;
			Idx = i;
		}
	}

	if (Lim + 1 >= TileCount || Lim - 1 < 0)
		return glm::ivec2(-1, -1);

	if (Horizontal && !SelectionPosIllegal(m_PlayerSteps[Idx].m_XPosition + 1, m_PlayerSteps[Idx].m_YPosition))
		return glm::ivec2(m_PlayerSteps[Idx].m_XPosition + 1, m_PlayerSteps[Idx].m_YPosition);
	else if (!Horizontal && !SelectionPosIllegal(m_PlayerSteps[Idx].m_XPosition, m_PlayerSteps[Idx].m_YPosition - 1))
		return glm::ivec2(m_PlayerSteps[Idx].m_XPosition, m_PlayerSteps[Idx].m_YPosition - 1);

	return glm::ivec2(-1, -1);
}

bool CGameManager::PositionOnBoardView(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	return x < WindowHeigth;
}

void CGameManager::HandleToucheEvent(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	if (GameScreenActive() && !CUIMessageBox::m_ActiveMessageBox)
	{
		bool OnBoardView = (x <= WindowHeigth);
		m_Dragged = true;
		m_LastTouchOnBoardView = OnBoardView;
		m_LastTouchX = x;
		m_LastTouchY = WindowHeigth - y;
		glm::uvec2 ClickedLetterPos = m_Renderer->GetTilePos(m_LastTouchX, m_LastTouchY);

		//placed letter has been clicked
		if ((m_PlayerStepIdxUndo = GetPlayerStepIdxAtPos(ClickedLetterPos.x, ClickedLetterPos.y)) != -1)
		{
			m_PlacedLetterTouchX = ClickedLetterPos.x;
			m_PlacedLetterTouchY = ClickedLetterPos.y;
		}
		else
			m_PlacedLetterTouchX = -1;
	}

	m_UIManager->HandleTouchEvent(x, WindowHeigth - y);

}

void CGameManager::HandleDragEvent(int x, int y)
{
	if (!GameScreenActive())
		return;

	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);
	y = WindowHeigth - y;

	if (m_TouchX == -1)
	{
		m_TouchX = x;
		m_TouchY = y;
	}

	if ((x - m_LastTouchX) * (x - m_LastTouchX) + (y - m_LastTouchY) * (y - m_LastTouchY) < 4) //TODO 4 dpi alapjan szamitval!
	    return;

	if (m_LastTouchOnBoardView)
		HandleDragFromBoardView(x, y);
	else
		HandleDragFromUIView(x, y);

	m_TouchX = x;
	m_TouchY = y;
}

void CGameManager::HandleDragFromUIView(int x, int y) 
{
	if (!GameScreenActive())
		return;

	//only let drag ui letter if previously dragged letter is placed
	if (m_Dragged && PlayerLetterAnimationFinished())
		m_UIManager->HandleDragEvent(x, y);
}

void CGameManager::HandleDragFromBoardView(int x, int y)
{
	if (!GameScreenActive())
		return;

	//rotate board
	if (m_Dragged && m_PlacedLetterTouchX == -1)
	{
		float ZRotAngle = float(x - m_TouchX) / 3.;
		float YRotAngle = float(m_TouchY - y) / 3.;

		m_Renderer->RotateCamera(-ZRotAngle, YRotAngle);
	}
	//placed letter back to dragged letter
	else if (m_Dragged)
	{
		int SelX = m_PlayerSteps.back().m_XPosition;
		int SelY = m_PlayerSteps.back().m_YPosition;

		RemovePlacedLetterSelAtPos(m_PlacedLetterTouchX, m_PlacedLetterTouchY);
		m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName())->SetLetterDragged(m_PlayerSteps[m_PlayerStepIdxUndo].m_LetterIdx, x, y);
		UndoStepAtPos(m_PlacedLetterTouchX, m_PlacedLetterTouchY);
		m_PlacedLetterTouchX = -1;
		m_LastTouchOnBoardView = false;
		m_Renderer->SelectField(SelX, SelY);
	}
}

void CGameManager::HandleZoomEndEvent()
{
	if (!GameScreenActive())
		return;

	m_Renderer->ResetZoom();
}

void CGameManager::HandleZoomEvent(float dist, int origoX, int origoY)
{
	if (!GameScreenActive())
		return;

	//zoom on ui view
	if (origoX > m_SurfaceHeigh)
		return;

	m_Dragged = false;
	m_Renderer->ZoomCameraCentered(dist, origoX, origoY);
}

void CGameManager::HandleZoomEvent(float dist)
{
	if (!GameScreenActive())
		return;

	m_Dragged = false;
	m_Renderer->ZoomCameraSimple(dist);
}


void CGameManager::HandleMultyDragEvent(int x0, int y0, int x1, int y1)
{
	if (!GameScreenActive())
		return;

    m_Dragged = false;
    m_Renderer->DragCamera(x0, y0, x1, y1);
}

void CGameManager::RenderPlacedLetterSelections()
{
	bool LastTile;
	bool FirstTile = true;

	for (size_t i = 0; i < m_PlacedLetterSelections.size(); ++i)
	{
		LastTile = i == m_PlacedLetterSelections.size() - 1;
		m_Renderer->DrawSelection(glm::vec4(0.98f, 0.9f, 0.39f, 0.5f), m_PlacedLetterSelections[i].x, m_PlacedLetterSelections[i].y, FirstTile, LastTile);
		FirstTile = false;
	}
}


void CGameManager::RenderTileAnimations()
{
	glm::ivec2 TilePos;
	glm::vec4 Color = m_TileAnimations->GetColor();
	bool LastTile = false;
	bool FirstTile = true;
	m_TileAnimations->StarDataQuery();

	while (m_TileAnimations->GetData(TilePos, LastTile))
	{
		m_Renderer->DrawSelection(Color, TilePos.x, TilePos.y, FirstTile, LastTile);
		FirstTile = false;
	}
}

void CGameManager::RenderUI()
{
	glDisable(GL_DEPTH_TEST);

	if (GetGameState() != EGameState::OnRankingsScreen)
	{
		m_UIManager->RenderUI();
		
		glEnable(GL_BLEND);
		m_UIManager->RenderDraggedLetter();
		glDisable(GL_BLEND);

		m_UIManager->RenderMessageBox();
	}
	else
		m_UIManager->RenderRankingsPanel();

	glEnable(GL_DEPTH_TEST);
}

void CGameManager::GoToStartGameScrEvent()
{
	SetGameState(CGameManager::OnStartGameScreen);
}

void CGameManager::EndPlayerTurnEvent()
{
	if (EndPlayerTurn())
	{
		m_PlacedLetterSelections.clear();
		m_UIManager->EnableGameButtons(false);
	}
}

void CGameManager::TopViewEvent()
{
	m_CameraAnimationManager->StartFitToScreenAnimation();
}

void CGameManager::RenderFrame()
{
	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);
	ShowFps &= ConfigFound;

	typedef std::chrono::high_resolution_clock Clock;
	
	if (m_Renderer && m_Renderer->IsInited())
	{
		if (ShowFps && frames == 0)
			LastRenderTime = Clock::now();
		
		{
			const std::lock_guard<std::recursive_mutex> lock(m_Renderer->GetRenderLock());

			if (GameScreenActive())
			{
				m_Renderer->Render();
				RenderTileAnimations();
				RenderPlacedLetterSelections();
			}
			else
				m_Renderer->ClearBuffers();

			RenderUI();
		}

		if (ShowFps)
		{
			std::chrono::high_resolution_clock::time_point RenderTime = Clock::now();
			std::chrono::duration<double, std::milli> TimeSpan = RenderTime - LastRenderTime;

			if (TimeSpan.count() != 0)
			{
				frames++;

				if (frames > 500)
				{
					float fps = (1000. / double(TimeSpan.count())) * double(frames);

					std::wstringstream ss;
					ss << L"fps : " << int(fps);
					m_UIManager->SetText(L"ui_fps_text", ss.str().c_str());
					frames = 0;
				}
			}
			else
			{
				frames++;
			}
		}
#ifndef PLATFORM_ANDROID
		OpenGLFunctions::SwapBuffers();
#endif
	}
}

