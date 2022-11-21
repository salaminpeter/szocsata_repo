#include "stdafx.h"
#include "GameState.h"
#include "GameBoard.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "UIManager.h"
#include "UIPlayerLetters.h"
#include "Config.h"
#include "UILayout.h"

#include<iostream>
#include<fstream>
#include <chrono>
#include <thread>

void CGameState::SaveGameState()
{	
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());

	if (m_GameManager->GetGameState() == CGameManager::OnStartScreen)
	{
		RemoveSaveFile();
		return;
	}

	std::string FilePath = m_GameManager->GetWorkingDir() + 
#ifdef PLATFORM_ANDROID
		"/state.dat";
#else
		"state.dat";
#endif
	
	std::ofstream StateFile(FilePath, std::ofstream::binary);

	if (!StateFile.good())
		return;

	int GameState = m_GameManager->GetGameState();
	int OnGameScreen = m_GameManager->GameScreenActive(static_cast<CGameManager::EGameState>(GameState));
	int PlayerCountIdx = m_GameManager->GetUIManager()->GetPlayerCount();
	int BoardSize = m_GameManager->GetUIManager()->GetBoardSize();
	int TimeLimit = m_GameManager->GetUIManager()->GetTimeLimitIdx();
	int Difficulty = m_GameManager->GetUIManager()->GetDifficulty();
	bool ComputerOpponentEnabled = m_GameManager->GetUIManager()->ComputerOpponentEnabled();

	StateFile.write((char *)&GameState, sizeof(int));

	if (!StateFile.good())
		return;

	StateFile.write((char *)&PlayerCountIdx, sizeof(int));

	if (!StateFile.good())
		return;

	StateFile.write((char *)&BoardSize, sizeof(int));

	if (!StateFile.good())
		return;

	StateFile.write((char *)&TimeLimit, sizeof(int));

	if (!StateFile.good())
		return;

	StateFile.write((char *)&ComputerOpponentEnabled, sizeof(bool));

	if (!StateFile.good())
		return;

	StateFile.write((char *)&Difficulty, sizeof(int));

	if (!StateFile.good())
		return;

	if (OnGameScreen) {

		bool HasTimeLimit = m_GameManager->GetUIManager()->GetTimeLimit() != -1;
		bool TimerRunning = m_GameManager->GetTimerEventManager()->IsTimerRunning("time_limit_event");
		StateFile.write((char *)&HasTimeLimit, sizeof(bool));

		if (HasTimeLimit)
		{
			StateFile.write((char *)&m_GameManager->m_RemainingTurnTime, sizeof(int));
			StateFile.write((char *)&TimerRunning, sizeof(bool));
		}

		size_t CurrentPlayerIdx = m_GameManager->GetCurrentPlayerIdx();
		StateFile.write((char *) &CurrentPlayerIdx, sizeof(size_t));

		if (!StateFile.good())
			return;

		m_GameManager->SetTileCount();
		int TileCount;
		CConfig::GetConfig("tile_count", TileCount);

		for (int i = 0; i < 2; ++i)
		{
			CGameBoard& Board = m_GameManager->GetBoard(i == 0);

			for (int x = 0; x < TileCount; ++x) {
				for (int y = 0; y < TileCount; ++y) {
					wchar_t chr = Board(x, y).m_Char;
					int height = Board(x, y).m_Height;

					StateFile.write((char *) &chr, sizeof(wchar_t));
					StateFile.write((char *) &height, sizeof(int));
				}
			}

			if (!StateFile.good())
				return;

		}

		int PlayerCount = PlayerCountIdx + 1 + (ComputerOpponentEnabled ? 1 : 0);

		for (size_t i = 0; i < PlayerCount; ++i) {
			int Score = m_GameManager->GetPlayer(i)->GetScore();
			StateFile.write((char *) &Score, sizeof(int));
			if (!StateFile.good())
				return;

			unsigned int UsedLetters = m_GameManager->GetPlayer(i)->GetUsedLetters().GetList();
			StateFile.write((char *) &UsedLetters, sizeof(unsigned int));
			if (!StateFile.good())
				return;

			std::wstring Letters = m_GameManager->GetPlayerLetters(i);
			std::wstring AllLetters = m_GameManager->GetPlayerLetters(i, true);
			int LetterCount = Letters.length();

			StateFile.write((char *) &LetterCount, sizeof(int));
			if (!StateFile.good())
				return;

			for (size_t j = 0; j < LetterCount; ++j)
				StateFile.write((char *) &Letters.at(j), sizeof(wchar_t));

			if (!StateFile.good())
				return;

			for (size_t j = 0; j < LetterCount; ++j)
				StateFile.write((char *) &AllLetters.at(j), sizeof(wchar_t));

			if (!StateFile.good())
				return;
		}

		const std::vector<TPlayerStep> &PlayerSteps = m_GameManager->GetPlayerSteps();
		int PlayerStepCount = PlayerSteps.size();
		StateFile.write((char *) &PlayerStepCount, sizeof(int));

		if (!StateFile.good())
			return;

		for (int i = PlayerStepCount - 1; i >= 0; --i) {
			StateFile.write((char *) &PlayerSteps[i].m_Char, sizeof(wchar_t));
			StateFile.write((char *) &PlayerSteps[i].m_LetterIdx, sizeof(int));
			StateFile.write((char *) &PlayerSteps[i].m_XPosition, sizeof(int));
			StateFile.write((char *) &PlayerSteps[i].m_YPosition, sizeof(int));
			if (!StateFile.good())
				return;

		}

		SaveComputerStep(StateFile);

		if (!StateFile.good())
			return;

		size_t CharCount = m_GameManager->GetCharacterCount();
		StateFile.write((char *) &CharCount, sizeof(size_t));
		if (!StateFile.good())
			return;

		for (size_t i = 0; i < CharCount; ++i) {
			size_t LetterCount = m_GameManager->GetLetterCount(i);
			StateFile.write((char *) &LetterCount, sizeof(size_t));
		}

		if (!StateFile.good())
			return;

		m_GameManager->SaveTileAnims(StateFile);
		if (!StateFile.good())
			return;

		m_GameManager->SaveWordAnims(StateFile);
		if (!StateFile.good())
			return;

		m_GameManager->SaveLetterAnims(StateFile);
		if (!StateFile.good())
			return;

		m_GameManager->SaveCamera(StateFile);
		if (!StateFile.good())
			return;

		m_GameManager->SaveScoreAnim(StateFile);
		if (!StateFile.good())
			return;

		m_GameManager->SavePopupState(StateFile);
		if (!StateFile.good())
			return;

	}

	StateFile.close();
}

void CGameState::SaveComputerStep(std::ofstream& fileStream)
{
	bool ComputersTurn = m_GameManager->GetCurrentPlayer()->IsComputer();
	fileStream.write((char *)&ComputersTurn, sizeof(bool));

	if (ComputersTurn)
	{
		CComputer* Computer = static_cast<CComputer*>(m_GameManager->GetCurrentPlayer());
		TComputerStep ComputerStep = Computer->GetComputerStep();
		bool HasComputerStep = ComputerStep.IsStepValid();

		fileStream.write((char *)&HasComputerStep, sizeof(bool));

		if (!fileStream.good())
			return;

		if (HasComputerStep)
			Computer->SaveComputerStep(fileStream);

		if (!fileStream.good())
			return;
	}
}

void CGameState::LoadComputerStep(std::ifstream& fileStream)
{
	bool ComputersTurn;
	fileStream.read((char *)&ComputersTurn, sizeof(bool));

	if (ComputersTurn)
	{
		CComputer* Computer = static_cast<CComputer*>(m_GameManager->GetCurrentPlayer());
		TComputerStep ComputerStep = Computer->GetComputerStep();
		bool HasComputerStep;

		fileStream.read((char *)&HasComputerStep, sizeof(bool));

		if (HasComputerStep)
			Computer->LoadComputerStep(fileStream);
	}
}


void CGameState::LoadPlayerAndBoardState()
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());

	std::string FilePath = m_GameManager->GetWorkingDir() + 
#ifdef PLATFORM_ANDROID
		"/state.dat";
#else
		"state.dat";
#endif

	std::ifstream StateFile(FilePath, std::ifstream::binary);
	StateFile.seekg(m_FilePos);

	if (!StateFile.good())
		return;

	float LetterHeight;
	float BoardHeight;

	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("letter_height", LetterHeight);

	int PlayerCountIdx = m_GameManager->GetUIManager()->GetPlayerCount();
	bool ComputerOpponentEnabled = m_GameManager->GetUIManager()->ComputerOpponentEnabled();

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

#ifndef PLATFORM_ANDROID
	#define size_t int64_t
	#define wchar_t char32_t
#endif
	if (m_GameManager->GameScreenActive(m_GameManager->m_SavedGameState))
	{
		bool HasTimeLimit;
		StateFile.read((char *)&HasTimeLimit, sizeof(bool));

		if (HasTimeLimit)
		{
			StateFile.read((char *)&m_GameManager->m_RemainingTurnTime, sizeof(int));
			StateFile.read((char *)&m_GameManager->m_CountDownRunning, sizeof(bool));
		}

		size_t CurrentPlayerIdx;
		StateFile.read((char *)&CurrentPlayerIdx, sizeof(size_t));

		CPlayer* CurrentPlayer = m_GameManager->GetPlayer(CurrentPlayerIdx);
		m_GameManager->SetCurentPlayer(CurrentPlayerIdx);

		for (int i = 0; i < 2; ++i)
		{
			CGameBoard& Board = m_GameManager->GetBoard(i == 0);

			for (int x = 0; x < TileCount; ++x) {
				for (int y = 0; y < TileCount; ++y) {
					wchar_t chr;
					int height;

					StateFile.read((char *)&chr, sizeof(wchar_t));
					StateFile.read((char *)&height, sizeof(int));

					Board(x, y).m_Char = chr;
					Board(x, y).m_Height = height;

					if (i == 1)
					{
						for (int h = 0; h < height; ++h)
							m_GameManager->AddLetterToBoard(x, TileCount - y - 1, chr, BoardHeight / 2.f + h * LetterHeight + LetterHeight / 2.f, true);
					}
				}
			}
		}

		int PlayerCount = PlayerCountIdx + 1 + (ComputerOpponentEnabled ? 1 : 0);

		for (size_t i = 0; i < PlayerCount; ++i)
		{
			int Score;
			StateFile.read((char *)&Score, sizeof(int));

			unsigned int UsedLetters;
			StateFile.read((char *)&UsedLetters, sizeof(unsigned int));
			m_GameManager->GetPlayer(i)->SetUsedLetters(UsedLetters);

			int LetterCount;
			StateFile.read((char *)&LetterCount, sizeof(int));
			std::wstring Letters(LetterCount, L' ');
			std::wstring AllLetters(LetterCount, L' ');

			for (size_t j = 0; j < LetterCount; ++j)
				StateFile.read((char *)&Letters.at(j), sizeof(wchar_t));

			for (size_t j = 0; j < LetterCount; ++j)
				StateFile.read((char *)&AllLetters.at(j), sizeof(wchar_t));

			m_GameManager->GetPlayer(i)->SetAllLetters(AllLetters.c_str());
			m_GameManager->GetPlayer(i)->AddScore(Score);
			m_GameManager->SetPlayerLetters(i, Letters);
			m_GameManager->GetUIManager()->GetPlayerLetters(i)->AddUILetters(LetterCount);
			m_GameManager->GetUIManager()->GetPlayerLetters(i)->ShowLetters(false);
		}

		const std::vector<TPlayerStep>& PlayerSteps = m_GameManager->GetPlayerSteps();
		int PlayerStepCount;
		StateFile.read((char *)&PlayerStepCount, sizeof(int));

		for (size_t i = 0; i < PlayerStepCount; ++i)
		{
			wchar_t Chr;
			int Idx;
			int XPos;
			int YPos;

			StateFile.read((char *)&Chr, sizeof(wchar_t));
			StateFile.read((char *)&Idx, sizeof(int));
			StateFile.read((char *)&XPos, sizeof(int));
			StateFile.read((char *)&YPos, sizeof(int));

			m_GameManager->AddPlayerStep(Chr, Idx, XPos, YPos);
			m_GameManager->AddPlacedLetterSelection(XPos, YPos);
		}

		LoadComputerStep(StateFile);

		size_t CharCount;
		StateFile.read((char *)&CharCount, sizeof(size_t));

		for (size_t i = 0; i < CharCount ; ++i)
		{
			size_t LetterCount;
			StateFile.read((char *)&LetterCount, sizeof(size_t));
			m_GameManager->SetLetterCount(i, LetterCount);
		}

		m_GameManager->CleanLetterPool();

		m_GameManager->SetTileCounterCount();

		std::wstring Name;
		int Score;
		glm::vec3 Color;

		m_GameManager->GetPlayerProperties(CurrentPlayerIdx, Name, Score, Color);
		m_GameManager->GetUIManager()->SetCurrentPlayerName(Name.c_str(), Color.r, Color.g, Color.b);
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->DiasbleLayoutPositioning(true);
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->SetLetterVisibility(CurrentPlayer->GetUsedLetters());
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->SetVisible(true);
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->DiasbleLayoutPositioning(false);

		m_GameManager->LoadTileAnims(StateFile);
		m_GameManager->LoadWordAnims(StateFile);
		m_GameManager->LoadLetterAnims(StateFile);
		m_GameManager->LoadCamera(StateFile);
		m_GameManager->LoadScoreAnim(StateFile);
		m_GameManager->LoadPopupState(StateFile);

		if (m_GameManager->m_SavedGameState != CGameManager::WaitingForMessageBox && CurrentPlayer->IsComputer())
		{
			CComputer* Computer = static_cast<CComputer*>(CurrentPlayer);

			//meg nem szamolta ki a computer a lepeset
			if (!Computer->GetComputerStep().IsStepValid())
				m_GameManager->StartPlayerTurn(CurrentPlayer, false);
		}
	}

	StateFile.close();

#ifndef PLATFORM_ANDROID
	#undef size_t
	#undef wchar_t
#endif
}

void CGameState::LoadGameState()
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());

	std::string FilePath = m_GameManager->GetWorkingDir() + 
#ifdef PLATFORM_ANDROID
		"/state.dat";
#else
		"state.dat";
#endif
	std::ifstream StateFile(FilePath, std::ifstream::binary);

	if (!StateFile.good())
		return;

	int GameState;
	int PlayerCountIdx;
	int BoardSize;
	int TimeLimit;
	int Difficulty;
	bool ComputerOpponentEnabled;

	StateFile.read((char *)&GameState, sizeof(int));
	std::ios_base::iostate State = StateFile.rdstate();
	bool eofbit = State == std::ios_base::eofbit;
	bool failbit = State == std::ios_base::failbit;
	bool badbit = State == std::ios_base::badbit;
	bool goodbit = State == std::ios_base::goodbit;
	StateFile.read((char *)&PlayerCountIdx, sizeof(int));
	StateFile.read((char *)&BoardSize, sizeof(int));
	StateFile.read((char *)&TimeLimit, sizeof(int));
	StateFile.read((char *)&ComputerOpponentEnabled, sizeof(bool));
	StateFile.read((char *)&Difficulty, sizeof(int));

	m_FilePos = StateFile.tellg();

    if (!StateFile.good())
        return;

	m_GameManager->GetUIManager()->SetPlayerCount(PlayerCountIdx);
	m_GameManager->GetUIManager()->SetBoardSize(BoardSize);
	m_GameManager->GetUIManager()->SetTimeLimitIdx(TimeLimit);
	m_GameManager->GetUIManager()->SetComputerOpponentEnabled(ComputerOpponentEnabled);
	m_GameManager->GetUIManager()->SetDifficulty(Difficulty);
	m_GameManager->SetTileCount();
	m_GameManager->SetBoardSize();
	m_GameManager->m_SavedGameState = static_cast<CGameManager::EGameState>(GameState);

	StateFile.close();
}

void CGameState::RemoveSaveFile()
{
	std::string FilePath = m_GameManager->GetWorkingDir() + 
#ifdef PLATFORM_ANDROID
		"/state.dat";
#else
		"state.dat";
#endif
	std::remove(FilePath.c_str());
}





