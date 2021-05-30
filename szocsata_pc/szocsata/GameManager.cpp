#include "stdafx.h"
#include "GameManager.h"
#include "Renderer.h"
#include "UIManager.h"
#include "TileAnimationManager.h"
#include "Config.h"
#include "GameBoard.h"
#include "UIButton.h"
#include "UIPlayerLetters.h"
#include "GridLayout.h"
#include "opengl.h"
#include "TimerEventManager.h"
#include "TileAnimationManager.h"
#include "WordAnimationManager.h"


//TODO computer jateknal preferalja a palya kozepe fele levo szvakat azonos pontszam eseten!!

CGameManager::CGameManager()
{
	CConfig::LoadConfigs("config.txt");

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	m_GameBoard.SetSize(TileCount);
	m_TmpGameBoard.SetSize(TileCount);
	CompGameBoard.SetSize(TileCount);

	m_DataBase.LoadDataBase("dic.txt");

	m_TimerEventManager = new CTimerEventManager();
	m_TileAnimations = new CTileAnimationManager(m_TimerEventManager, this);
	m_WordAnimation = new CWordAnimationManager(m_TimerEventManager, this); //TODO!!!!!!!!!!!
}


void CGameManager::AddPlayers(int playerCount, bool addComputer)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	for (int i = 0; i < playerCount; ++i)
	{
		m_Players.push_back(new CPlayer(this));
		m_LetterPool.DealLetters(m_Players.back()->GetLetters());
		m_UIManager->AddPlayerLetters(m_Players.back()->GetName().c_str(), m_Players.back()->GetLetters().c_str(), m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorGridData8x4(), "view_ortho");
		PositionPlayerLetters(m_Players.back()->GetName().c_str());
		m_UIManager->GetPlayerLetters(m_Players.back()->GetName().c_str())->ShowLetters(false);
	}

	if (addComputer)
	{ 
		m_Computer = new CComputer(this);
		m_LetterPool.DealLetters(m_Computer->GetLetters());
		m_Players.push_back(m_Computer);
		m_UIManager->AddPlayerLetters(m_Players.back()->GetName().c_str(), m_Computer->GetLetters().c_str(), m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorGridData8x4(), "view_ortho");
		PositionPlayerLetters(m_Players.back()->GetName().c_str());
		m_UIManager->GetPlayerLetters(m_Players.back()->GetName().c_str())->ShowLetters(false);
	}
}

void CGameManager::StartGame()
{
	m_GameEnded = false;
	m_LetterPool.Init();
	AddPlayers(1, true);
	UpdatePlayerScores();
	StartPlayerTurn(m_Players[0]);
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

	CUIPlayerLetters* UIPlayerLetters = m_UIManager->GetPlayerLetters(player->GetName().c_str());
	UIPlayerLetters->SetLetterVisibility();
	UIPlayerLetters->SetVisible(true);

	player->ResetUsedLetters();

	m_FirstPlayerLetterX = -1;
	m_FirstPlayerLetterY = -1;
	m_SecondPlayerLetterX = -1;
	m_SecondPlayerLetterY = -1;

	m_CurrentPlayer = player;
	m_CurrentPlayer->m_TurnInProgress = true;

	SetGameState(EGameState::TurnInProgress);

	m_TmpGameBoard = m_GameBoard;
}


void CGameManager::NextPlayerTurn()
{
	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->ShowLetters(false);

	int CurrPlayerIdx = -1;

	while (m_Players[++CurrPlayerIdx] != m_CurrentPlayer);

	int NextPlayerIdx = CurrPlayerIdx == m_Players.size() - 1 ? 0 : CurrPlayerIdx + 1;

	//letette e az osszes betujet a jatekos
	if (m_CurrentPlayer->GetLetterCount() == 0)
	{
		SetGameState(EGameState::GameEnded);
		m_GameEnded = true;
		return;
	}

	//ha mindenki passzolt akkor vege
	m_GameEnded = true;
	for (size_t i = 0; i < m_Players.size(); ++i)
		m_GameEnded &= m_Players[i]->m_Passed;
	
	if (m_GameEnded)
	{
		SetGameState(EGameState::GameEnded);
		return;
	}

	m_Players[NextPlayerIdx]->m_Passed = false;
	SetGameState(EGameState::TurnInProgress);

	if (m_Players[NextPlayerIdx]->GetName() == L"Computer")
		StartComputerturn();
	else
		StartPlayerTurn(m_Players[NextPlayerIdx]);
}


bool CGameManager::EndPlayerTurn()
{	
	m_CurrentPlayer->m_TurnInProgress = false;

	m_Renderer->HideSelection(true);

	//jatekos passz
	if (m_CurrentPlayer->GetUsedLetterCount() == 0)
	{
		SetGameState(EGameState::PlayerPass);
		m_CurrentPlayer->m_Passed = true;
		NextPlayerTurn();
		return false;
	}

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	bool Horizontal = m_FirstPlayerLetterY == m_SecondPlayerLetterY;
	bool HorizUndefined = m_SecondPlayerLetterY == -1; //ha csak egy betut tettunk le
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

	AddWordSelectionAnimation(CrossingWords, true);
	m_CurrentPlayer->AddScore(Score);
	m_LetterPool.DealLetters(m_CurrentPlayer->GetLetters());
	
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
	PlayerLetters->SetLetters(m_CurrentPlayer->GetLetters());
	PlayerLetters->SetLetterVisibility();

	m_CurrentPlayer->ResetUsedLetters();

	UpdatePlayerScores();
	SetGameState(EGameState::WaintingOnAnimation);

	return true;
}

void CGameManager::DealComputerLetters()
{
	m_LetterPool.DealLetters(m_Computer->GetLetters());
	CUIPlayerLetters* ComputerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
	ComputerLetters->SetLetters(m_CurrentPlayer->GetLetters());
	ComputerLetters->SetLetterVisibility();
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
	}

	//computer passz
	if (ComputerPass || !ComputerWord.m_Word)
	{
		SetGameState(EGameState::ComputerPass);
		m_CurrentPlayer->m_Passed = true;
		NextPlayerTurn();
		return false;
	}

	if (PrevTickCount == 0)
		PrevTickCount = CTimer::GetCurrentTime();

	bool LetterAdded = false;
	int ComputerStepDelay;

	CConfig::GetConfig("computer_step_delay", ComputerStepDelay);

	std::vector<size_t> LetterIndices;
	size_t WordLength = ComputerWord.m_Word->length();
	LetterIndices.reserve(WordLength);

	for (size_t i = 0; i < WordLength; ++i)
		LetterIndices.push_back(m_Computer->RemoveLetter(ComputerWord.m_Word->at(i)));

	m_WordAnimation->AddWordAnimation(*ComputerWord.m_Word, LetterIndices, ComputerWord.m_X, TileCount - ComputerWord.m_Y - 1, ComputerWord.m_Horizontal);
	m_GameBoard.AddWord(ComputerWord);

	/*
	if (CTimer::GetCurrentTime() - PrevTickCount > ComputerStepDelay)
	{
		for (size_t i = LetterIdx; i < ComputerWord.m_Word->length(); ++i)
		{ 
			wchar_t CharOnBoard;
			wchar_t PlacedLetter = (*ComputerWord.m_Word)[i];

			if (ComputerWord.m_Horizontal)
				CharOnBoard = m_GameBoard(ComputerWord.m_X + i, ComputerWord.m_Y).m_Char;
			else
				CharOnBoard = m_GameBoard(ComputerWord.m_X, ComputerWord.m_Y + i).m_Char;

			if (CharOnBoard != PlacedLetter)
			{
				int x = ComputerWord.m_X + (ComputerWord.m_Horizontal ? i : 0);
				int y = ComputerWord.m_Y + (ComputerWord.m_Horizontal ? 0 : i);

				LettersAdded++;
				m_Renderer->AddLetterToBoard(x, TileCount - y - 1, PlacedLetter);

				m_Computer->RemoveLetter(PlacedLetter);
				int RemovedIdx = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName())->RemoveLetter(PlacedLetter);
				m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName())->SetVisible(false, RemovedIdx);

				LetterIdx = i + 1;
				LetterAdded = true;
				PrevTickCount = CTimer::GetCurrentTime();
				break;
			}
		}

		if (LettersAdded == m_Computer->GetUsedLetterCount())
		{
			m_GameBoard.AddWord(ComputerWord);

			m_LetterPool.DealLetters(m_Computer->GetLetters());
			CUIPlayerLetters* ComputerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
			ComputerLetters->SetLetters(m_CurrentPlayer->GetLetters());
			ComputerLetters->SetLetterVisibility();

			PrevTickCount = 0;
			LetterIdx = 0;
			LettersAdded = 0;
			m_CurrentPlayer->ResetUsedLetters();
			m_ComputerWordIdx = -1;
			UpdatePlayerScores();
			SetGameState(EGameState::ComputerTurnEnd);

			return true;
		}

		return LetterAdded;
	}
	*/
	return false;
}

void CGameManager::StartComputerturn()
{
	m_Computer->m_TurnInProgress = true;

	m_UIManager->GetPlayerLetters(m_Computer->GetName().c_str())->SetLetterVisibility();
	m_UIManager->GetPlayerLetters(m_Computer->GetName().c_str())->SetVisible(true);

	//	CompGameBoard = m_GameBoard;
	//	CompLetters = m_Computer->m_Letters;	
	m_TmpGameBoard = m_GameBoard;

	m_CurrentPlayer = m_Computer;
	m_Computer->CalculateStep();

	int GameDifficulty;
	int BestWordCount = m_Computer->BestWordCount();

	CConfig::GetConfig("game_difficulty", GameDifficulty);

	int WordCount;

	if (GameDifficulty == 2)
		WordCount = 1;
	else
		WordCount = BestWordCount;

	m_ComputerWordIdx = std::rand() / ((RAND_MAX + 1u) / WordCount);

	if (m_Computer->BestWordCount() && m_ComputerWordIdx < m_Computer->BestWordCount())
	{
		TComputerStep ComputerStep = m_Computer->BestWord(m_ComputerWordIdx);

		m_Computer->AddScore(ComputerStep.m_Score);
		m_Computer->SetUsedLetters(ComputerStep.m_UsedLetters);
	}
	else
	{
		//coputer pass
		m_Computer->m_Passed = true;
	}

	m_Computer->m_TurnInProgress = false;

	EndComputerTurn();
}

void CGameManager::UpdatePlayerScores()
{
	std::wstringstream cs;
	cs << L"computer : " << m_Players.back()->GetScore();// << L"..";
	m_UIManager->SetText(L"ui_computer_score", cs.str().c_str());

	std::wstringstream ps;
	ps << L"jatekos : " << m_Players[0]->GetScore();// << L"..";
	m_UIManager->SetText(L"ui_player_score", ps.str().c_str());
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

	if (!m_GameEnded)
	{
		if (GetGameState() == EGameState::NextTurn)
		{
			NextPlayerTurn();
			SetGameState(EGameState::None);
		}

		m_TimerEventManager->Loop();
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
	int CrossingWordCount = 0;
	
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

		if (!SingleHeightFound)
			return 0;

		(*crossingWords).emplace_back(nullptr, word.m_X, word.m_Y, true, word.m_Word->length());

		CrossingWordCount++;

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
					return 0;

				Score++;

				(*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());

				CrossingWordCount++;
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

		if (!SingleHeightFound)
			return 0;

		(*crossingWords).emplace_back(nullptr, word.m_X, word.m_Y, false, word.m_Word->length());

		CrossingWordCount++;

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
					return 0;

				Score++;

				(*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());

				CrossingWordCount++;
			}
		}
	}

	return CrossingWordsValid ? Score : 0;
}

void CGameManager::SetTopView() 
{ 
	m_Renderer->SetTopView(true); 
}

void CGameManager::PlayerLetterClicked(unsigned letterIdx)
{
	int SelX, SelY;
	m_Renderer->GetSelectionPos(SelX, SelY);

	if (SelX == -1 || SelY == -1)
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
	m_WordAnimation->AddWordAnimation(std::wstring(1, PlacedLetter), std::vector<size_t>{letterIdx}, SelX, SelY, true, false);
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
	PlayerLetters->RemoveLetter(size_t(letterIdx));
	PlayerLetters->SetVisible(false, letterIdx);

	m_PlayerSteps.emplace_back(PlacedLetter, SelX, SelY, letterIdx);

	m_CurrentPlayer->SetLetterUsed(letterIdx, true);

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


void CGameManager::PositionPlayerLetters(const std::wstring& playerId)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	for (size_t i = 0; i < LetterCount; ++i)
	{
		auto GridPos = m_GridLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left + Size / 2;
		float YPos = GridPos.m_Bottom - Size / 2;

		m_UIManager->PositionPlayerLetter(playerId.c_str(), i, XPos, YPos + 10, Size);
	}
}

void CGameManager::InitLayouts()
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	m_GridLayout = new CGridLayout(m_SurfaceHeigh, m_SurfaceHeigh / 2, m_SurfaceWidth - m_SurfaceHeigh, m_SurfaceHeigh / 3, 50.f, 60.f);
	m_GridLayout->AllignGrid(LetterCount, true);
}

void CGameManager::InitUIManager()
{
	m_UIManager = new CUIManager(this);
	m_UIManager->InitUI(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData8x8());
}


void CGameManager::InitRenderer(int surfaceWidth, int surfaceHeight)
{
	m_SurfaceWidth = surfaceWidth;
	m_SurfaceHeigh = surfaceHeight;

	m_Renderer = new CRenderer(surfaceWidth, surfaceHeight, this);
	m_Renderer->Init();
}

glm::vec2 CGameManager::GetViewPosition(const char* viewId) 
{ 
	return m_Renderer->GetViewPosition(viewId); 
}

void CGameManager::UpdateBoardAnimation()
{
	if (m_Renderer)
		m_Renderer->UpdateBoardAnimation();
}


void CGameManager::HandleReleaseEvent(int x, int y)
{
    m_TouchX = -1;

	if (m_LastTouchOnBoardView)
		HandleReleaseEventFromBoardView(x, y);
	else
		HandleReleaseEventFromUIView(x, y);
}

void CGameManager::HandleReleaseEventFromBoardView(int x, int y)
{
//	m_Renderer->ResetZoom();
	m_Renderer->CalculateScreenSpaceGrid();
	m_Dragged = false;

	float Offset = std::sqrtf((m_LastTouchX - x) * (m_LastTouchX - x) + (m_LastTouchY - y) * (m_LastTouchY - y));

	if (Offset > 3) //TODO configbol
		return;

	TPosition p = m_Renderer->GetTilePos(m_LastTouchX, m_SurfaceHeigh - m_LastTouchY);

	if (p.x != -1)
		m_Renderer->SelectField(p.x, p.y);
}

void CGameManager::HandleReleaseEventFromUIView(int x, int y)
{
	m_Dragged = false;
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

	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetters(m_CurrentPlayer->GetLetters());
	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility();
	
	m_Renderer->RemoveLastLetter();
//	m_Renderer->SetLetterVisibility(true, m_CurrentPlayer->GetName(), m_PlayerSteps.back().m_LetterIdx);
	m_PlayerSteps.pop_back();
}


void CGameManager::HandleToucheEvent(int x, int y, bool onBoardView)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	m_UIManager->HandleTouchEvent(x, WindowHeigth - y);

	m_Dragged = true;
	m_LastTouchOnBoardView = onBoardView;
	m_LastTouchX = x;
	m_LastTouchY = y;
}

void CGameManager::HandleDragEvent(int x, int y)
{
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

void CGameManager::HandleDragFromBoardView(int x, int y)
{
	if (m_Dragged)
	{
		float ZRotAngle = float(x - m_TouchX) / 3.;
		float YRotAngle = float(y - m_TouchY) / 3.;

		m_Renderer->RotateCamera(-ZRotAngle, -YRotAngle);
	}
}

void CGameManager::HandleZoomEndEvent()
{
	m_Renderer->ResetZoom();
}

void CGameManager::HandleZoomEvent(float dist, int origoX, int origoY)
{
	//zoom on ui view
	if (origoX > m_SurfaceHeigh)
		return;

	m_Dragged = false;
	m_Renderer->ZoomCamera(dist, origoX, origoY, 0);
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
	size_t idx = 0;

	m_UIManager->RenderTexts();
	m_UIManager->RenderButtons();

	if (m_CurrentPlayer)
		m_UIManager->RenderPlayerLetters(m_CurrentPlayer->GetName().c_str());
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
	m_Renderer->SetTopView(true);
}

void CGameManager::RenderFrame()
{
	typedef std::chrono::high_resolution_clock Clock;
	
	if (m_Renderer && m_Renderer->IsInited())
	{
		if (frames == 0)
			LastRenderTime = Clock::now();
		
		m_Renderer->Render();
		RenderTileAnimations();
		RenderUI();

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

