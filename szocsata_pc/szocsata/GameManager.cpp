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
	m_UIManager->ShowMessageBox(CUIMessageBox::Ok, m_Players[0]->GetName().c_str());
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
	m_PlayerSteps.clear();

	m_FirstPlayerLetterX = -1;
	m_FirstPlayerLetterY = -1;
	m_SecondPlayerLetterX = -1;
	m_SecondPlayerLetterY = -1;

	m_CurrentPlayer = player;

	SetGameState(EGameState::TurnInProgress);

	m_TmpGameBoard = m_GameBoard;
}

bool CGameManager::TileAnimationFinished() 
{ 
	return m_TileAnimations->Finished(); 
}

bool CGameManager::PlayerLetterAnimationFinished() 
{ 
	return m_PlayerLetterAnimationManager->Finished(); 
}

void CGameManager::ShowNextPlayerPopup()
{
	m_UIManager->ShowMessageBox(CUIMessageBox::Ok, GetNextPlayerName().c_str());
	m_UIManager->EnableGameButtons(true);
}


bool CGameManager::GameScreenActive()
{
	return (GetGameState() != EGameState::OnRankingsScreen && GetGameState() != EGameState::OnStartScreen);
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
	int CurrPlayerIdx = -1;
	while (m_Players[++CurrPlayerIdx] != m_CurrentPlayer);
	int NextPlayerIdx = CurrPlayerIdx == m_Players.size() - 1 ? 0 : CurrPlayerIdx + 1;
	
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

	//TODO biztos passzolni akarsz msgbox!
	m_UIManager->ShowToast(L"passz", AllPassed);
}

bool CGameManager::EndPlayerTurn()
{	
	m_Renderer->HideSelection(true);

	//jatekos passz
	if (m_CurrentPlayer->GetUsedLetterCount() == 0)
	{
		HandlePlayerPass();
		return false;
	}

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	bool HorizUndefined = (m_SecondPlayerLetterX == -1); //ha csak egy betut tettunk le
	bool Horizontal = !HorizUndefined && m_FirstPlayerLetterY == m_SecondPlayerLetterY;
	int BoardX = m_FirstPlayerLetterX;
	int BoardY = TileCount - m_FirstPlayerLetterY - 1;

	std::wstring PlayerWord;

	if (!HorizUndefined)
		PlayerWord = GetWordAtPos(Horizontal, BoardX, BoardY);
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

	if (PlayerWord.empty() || !m_DataBase.WordExists(PlayerWord)) //TODO miert lehet PlayerWord.empty()!!!!
	{ 
		CrossingWords.push_back(WordPos);
		AddWordSelectionAnimation(CrossingWords, false);
		return false;
	}

	CrossingWords.reserve(PlayerWord.length());
	int Score = CalculateScore(WordPos, &CrossingWords);

	if (Score == 0)
	{
		AddWordSelectionAnimation(CrossingWords, false);
		return false;
	}

	DealCurrPlayerLetters();
	AddWordSelectionAnimation(CrossingWords, true);
	m_UIManager->SetTileCounterValue(m_LetterPool.GetRemainingLetterCount());

	m_CurrentPlayer->AddScore(Score);
	UpdatePlayerScores();
	SetGameState(EGameState::WaintingOnAnimation);

	return true;
}

void CGameManager::DealCurrPlayerLetters()
{
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
	m_LetterPool.DealLetters(m_CurrentPlayer->GetLetters());
	m_CurrentPlayer->GetLetters().erase(remove(m_CurrentPlayer->GetLetters().begin(), m_CurrentPlayer->GetLetters().end(), ' '), m_CurrentPlayer->GetLetters().end());
	PlayerLetters->SetLetters();

	m_UIManager->PositionPlayerLetters(m_CurrentPlayer->GetName().c_str());
	PlayerLetters->OrderLetterElements();

	for (size_t i = 0; i < m_CurrentPlayer->GetLetters().length(); ++i)
	{
		if (m_CurrentPlayer->LetterUsed(i))
		{
			PlayerLetters->GetChild(i)->Scale(0.f);
			m_PlayerLetterAnimationManager->AddAnimation(PlayerLetters->GetChild(i), PlayerLetters->GetChild(i)->GetWidth());
		}
	}

	PlayerLetters->SetLetterVisibility(CBinaryBoolList());
	m_PlayerLetterAnimationManager->StartAnimations();
	m_CurrentPlayer->ResetUsedLetters();

}

void CGameManager::DealComputerLettersEvent()
{
	DealCurrPlayerLetters();
	m_UIManager->SetTileCounterValue(m_LetterPool.GetRemainingLetterCount());
}

bool CGameManager::EndComputerTurn()
{
	static unsigned long PrevTickCount = 0;
	static int LetterIdx = 0;
	static int LettersAdded = 0;

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

	if (PrevTickCount == 0)
		PrevTickCount = CTimer::GetCurrentTime();

	bool LetterAdded = false;
	int ComputerStepDelay;

	CConfig::GetConfig("computer_step_delay", ComputerStepDelay);

	size_t WordLength = ComputerWord.m_Word->length();

	std::vector<size_t> LetterIndices = m_CurrentPlayer->GetLetterIndicesForWord(*ComputerWord.m_Word);
	m_WordAnimation->AddWordAnimation(*ComputerWord.m_Word, LetterIndices, m_UIManager->GetPlayerLetters(m_Computer->GetName()), ComputerWord.m_X, TileCount - ComputerWord.m_Y - 1, ComputerWord.m_Horizontal);
	m_GameBoard.AddWord(ComputerWord);
	UpdatePlayerScores();

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

	if (m_PlayerSteps.size() == 1 && SelX != m_FirstPlayerLetterX && SelY != m_FirstPlayerLetterY)
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

	bool Horizontal = m_FirstPlayerLetterY == m_SecondPlayerLetterY;

	if (m_FirstPlayerLetterX == -1)
	{
		m_FirstPlayerLetterX = SelX;
		m_FirstPlayerLetterY = SelY;
	}
	else if (m_SecondPlayerLetterX == -1)
	{
		m_SecondPlayerLetterX = SelX;
		m_SecondPlayerLetterY = SelY;
		Horizontal = m_FirstPlayerLetterY == m_SecondPlayerLetterY;
	}
	else if (Horizontal && SelY != m_FirstPlayerLetterY || !Horizontal && SelX != m_FirstPlayerLetterX)
		return;

	wchar_t PlacedLetter = m_CurrentPlayer->GetLetters()[letterIdx];
	m_WordAnimation->AddWordAnimation(std::wstring(1, PlacedLetter), std::vector<size_t>{letterIdx}, m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName()), SelX, SelY, true, false);
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());

	m_PlayerSteps.emplace_back(PlacedLetter, SelX, SelY, letterIdx);

	int BoardY = TileCount - SelY - 1;
	m_GameBoard(SelX, BoardY).m_Char = PlacedLetter;
	m_GameBoard(SelX, BoardY).m_Height++;

	if (m_SecondPlayerLetterX != -1 && (Horizontal && SelX < TileCount - 1 || !Horizontal && SelY > 0))
	{
		SelX += Horizontal ? 1 : 0;
		SelY -= Horizontal ? 0 : 1;
		m_Renderer->HideSelection(false);
		m_Renderer->SelectField(SelX, SelY);
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
	m_UIManager = new CUIManager(this, m_TimerEventManager);
	m_UIManager->InitUIElements(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData8x8(), m_Renderer->GetSquareColorGridData8x4());
	m_UIManager->SetText(L"ui_fps_text", L"fps : 0");
	m_UIManager->SetTileCounterValue(m_LetterPool.GetRemainingLetterCount());
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
	if (GetGameState() == OnStartScreen)
		return;

	m_Renderer->CalculateScreenSpaceGrid();
	m_Dragged = false;

	float Offset = std::sqrtf((m_LastTouchX - x) * (m_LastTouchX - x) + (m_LastTouchY - y) * (m_LastTouchY - y));

	if (Offset > 3) //TODO configbol
		return;

	TPosition p = m_Renderer->GetTilePos(m_LastTouchX, m_LastTouchY);

	if (p.x != -1)
		m_Renderer->SelectField(p.x, p.y);
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

	StartPlayerTurn(m_CurrentPlayer);
}

void CGameManager::UndoLastStep()
{
	if (m_PlayerSteps.size() == 0)
		return;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);
	
	if (m_PlayerSteps.size() == 2)
		m_SecondPlayerLetterX = -1;
	else if (m_PlayerSteps.size() == 1)
		m_FirstPlayerLetterX = -1;

	int BoardY = TileCount - m_PlayerSteps.back().m_YPosition - 1;

	m_GameBoard(m_PlayerSteps.back().m_XPosition, BoardY).m_Char = m_TmpGameBoard(m_PlayerSteps.back().m_XPosition, BoardY).m_Char;
	m_GameBoard(m_PlayerSteps.back().m_XPosition, BoardY).m_Height--;

	m_CurrentPlayer->SetLetter(m_PlayerSteps.back().m_LetterIdx, m_PlayerSteps.back().m_Char);
	m_CurrentPlayer->SetLetterUsed(m_PlayerSteps.back().m_LetterIdx, false);

	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility(m_CurrentPlayer->GetUsedLetters());
	
	m_Renderer->RemoveLastLetter();
	m_PlayerSteps.pop_back();
}

bool CGameManager::PositionOnBoardView(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	return x < WindowHeigth;
}

void CGameManager::HandleToucheEvent(int x, int y, bool onBoardView)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	m_UIManager->HandleTouchEvent(x, WindowHeigth - y);

	if (GameScreenActive())
	{
		m_Dragged = true;
		m_LastTouchOnBoardView = onBoardView;
		m_LastTouchX = x;
		m_LastTouchY = WindowHeigth - y;
	}
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

	if (m_Dragged)
	{
		float ZRotAngle = float(x - m_TouchX) / 3.;
		float YRotAngle = float(m_TouchY - y) / 3.;

		m_Renderer->RotateCamera(-ZRotAngle, YRotAngle);
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
	if (GetGameState() == OnStartScreen)
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
		m_UIManager->RenderMessageBox();
	}
	else
		m_UIManager->RenderRankingsPanel();

	glEnable(GL_DEPTH_TEST);
}

void CGameManager::EndPlayerTurnEvent()
{
	if (EndPlayerTurn())
		m_UIManager->EnableGameButtons(false);
}

void CGameManager::BackSpaceEvent()
{
	if (m_PlayerSteps.size() == 0)
		return;
	
	float PrevX = m_PlayerSteps.back().m_XPosition;
	float PrevY = m_PlayerSteps.back().m_YPosition;
	
	UndoLastStep();

	m_Renderer->SelectField(PrevX, PrevY);
}

void CGameManager::TopViewEvent()
{
	m_CameraAnimationManager->StartFitToScreenAnimation();
}

void CGameManager::RenderFrame()
{
	typedef std::chrono::high_resolution_clock Clock;
	
	if (m_Renderer && m_Renderer->IsInited())
	{
		if (frames == 0)
			LastRenderTime = Clock::now();
		
		{
			const std::lock_guard<std::recursive_mutex> lock(m_Renderer->GetRenderLock());

			if (GetGameState() != OnStartScreen && GetGameState() != OnRankingsScreen)
			{
				m_Renderer->Render();
				RenderTileAnimations();
			}
			else
				m_Renderer->ClearBuffers();

			RenderUI();
		}

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
				//OutputDebugString(str);
				frames = 0;
			}
		}
		else
		{
			frames++;
		}
#ifndef PLATFORM_ANDROID
		OpenGLFunctions::SwapBuffers();
#endif
	}
}

