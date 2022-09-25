#include "stdafx.h"
#include "GameState.h"
#include "GameBoard.h"
#include "GameManager.h"
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

	if (StateFile.fail())
		return;

	int GameState = m_GameManager->GetGameState();
	int OnGameScreen = m_GameManager->GameScreenActive(static_cast<CGameManager::EGameState>(GameState));
	int PlayerCountIdx = m_GameManager->GetUIManager()->GetPlayerCount();
	int BoardSize = m_GameManager->GetUIManager()->GetBoardSize();
	int TimeLimit = m_GameManager->GetUIManager()->GetTimeLimitIdx();
	int Difficulty = m_GameManager->GetUIManager()->GetDifficulty();
	bool ComputerOpponentEnabled = m_GameManager->GetUIManager()->ComputerOpponentEnabled();

	StateFile.write((char *)&GameState, sizeof(int));
	StateFile.write((char *)&PlayerCountIdx, sizeof(int));
	StateFile.write((char *)&BoardSize, sizeof(int));
	StateFile.write((char *)&TimeLimit, sizeof(int));
	StateFile.write((char *)&ComputerOpponentEnabled, sizeof(bool));
	StateFile.write((char *)&Difficulty, sizeof(int));

	if (OnGameScreen)
	{
		size_t CurrentPlayerIdx = m_GameManager->GetCurrentPlayerIdx();
		StateFile.write((char *)&CurrentPlayerIdx, sizeof(size_t));

		m_GameManager->SetTileCount();
		int TileCount;
		CConfig::GetConfig("tile_count", TileCount);

		for (int x = 0; x < TileCount; ++x)
		{
			for (int y = 0; y < TileCount; ++y)
			{
				wchar_t chr = m_GameManager->TmpBoard(x, y).m_Char;
				int height = m_GameManager->TmpBoard(x, y).m_Height;

				StateFile.write((char *)&chr, sizeof(wchar_t));
				StateFile.write((char *)&height, sizeof(int));
			}
		}

        int PlayerCount = PlayerCountIdx + 1 + (ComputerOpponentEnabled ? 1 : 0);

        for (size_t i = 0; i < PlayerCount; ++i)
		{
			int Score = m_GameManager->GetPlayer(i)->GetScore();
			StateFile.write((char *)&Score, sizeof(int));

        	unsigned int UsedLetters = m_GameManager->GetPlayer(i)->GetUsedLetters().GetList();
			StateFile.write((char *)&UsedLetters, sizeof(unsigned int));

			std::wstring Letters = m_GameManager->GetPlayerLetters(i);
            std::wstring AllLetters = m_GameManager->GetPlayerLetters(i, true);
			int LetterCount = Letters.length();

			StateFile.write((char *)&LetterCount, sizeof(int));

			for (size_t j = 0; j < LetterCount; ++j)
				StateFile.write((char *)&Letters.at(j), sizeof(wchar_t));

            for (size_t j = 0; j < LetterCount; ++j)
                StateFile.write((char *)&AllLetters.at(j), sizeof(wchar_t));
        }

		const std::vector<TPlayerStep>& PlayerSteps = m_GameManager->GetPlayerSteps();
        int PlayerStepCount = PlayerSteps.size();
		StateFile.write((char *)&PlayerStepCount, sizeof(int));

		for (int i = PlayerStepCount - 1; i >= 0 ; --i)
		{
			StateFile.write((char *)&PlayerSteps[i].m_Char, sizeof(wchar_t));
			StateFile.write((char *)&PlayerSteps[i].m_LetterIdx, sizeof(int));
			StateFile.write((char *)&PlayerSteps[i].m_XPosition, sizeof(int));
			StateFile.write((char *)&PlayerSteps[i].m_YPosition, sizeof(int));
		}
	}

	size_t CharCount = m_GameManager->GetCharacterCount();
	StateFile.write((char *)&CharCount, sizeof(size_t));

	for (size_t i = 0; i < CharCount ; ++i)
	{
		size_t LetterCount = m_GameManager->GetLetterCount(i);
		StateFile.write((char *)&LetterCount, sizeof(size_t));
	}

	
	StateFile.close();
}

void CGameState::LoadPlayerAndBoardState()
{
	std::string FilePath = m_GameManager->GetWorkingDir() + 
#ifdef PLATFORM_ANDROID
		"/state.dat";
#else
		"state.dat";
#endif

	std::ifstream StateFile(FilePath, std::ifstream::binary);
	StateFile.seekg(m_FilePos);

	if (StateFile.fail())
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
	
	size_t CurrentPlayerIdx;
	StateFile.read((char *)&CurrentPlayerIdx, sizeof(size_t));

	for (int x = 0; x < TileCount; ++x) {
		for (int y = 0; y < TileCount; ++y) {
			wchar_t chr;
			int height;

			StateFile.read((char *)&chr, sizeof(wchar_t));
			StateFile.read((char *)&height, sizeof(int));

			m_GameManager->TmpBoard(x, y).m_Char = chr;
			m_GameManager->TmpBoard(x, y).m_Height = height;

			for (int h = 0; h < height; ++h)
				m_GameManager->AddLetterToBoard(x, TileCount - y - 1, chr, BoardHeight / 2.f + h * LetterHeight + LetterHeight / 2.f);
		}
	}

	m_GameManager->RevertGameBoard();

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
        m_GameManager->SetPlayerLetters(i, AllLetters, true);
		m_GameManager->GetUIManager()->GetPlayerLetters(i)->AddUILetters(LetterCount, true);
		m_GameManager->GetUIManager()->GetPlayerLetters(i)->ShowLetters(false);
		m_GameManager->GetUIManager()->GetPlayerLetters(i)->SetVisible(false);
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
        m_GameManager->AddLetterToBoard(XPos, YPos, Chr, BoardHeight / 2.f + m_GameManager->Board(XPos, TileCount - YPos - 1).m_Height * LetterHeight + LetterHeight / 2.f);
		m_GameManager->Board(XPos, TileCount - YPos - 1).m_Height++;
    }

	size_t CharCount;
	StateFile.read((char *)&CharCount, sizeof(size_t));

	for (size_t i = 0; i < CharCount ; ++i)
	{
		size_t LetterCount;
		StateFile.read((char *)&LetterCount, sizeof(size_t));
		m_GameManager->SetLetterCount(i, LetterCount);
	}

	m_GameManager->SetTileCounterCount();

	if (m_GameManager->GameScreenActive(m_GameManager->m_SavedGameState))
	{
		std::wstring Name;
		int Score;
		glm::vec3 Color;

		CPlayer* CurrentPlayer = m_GameManager->GetPlayer(CurrentPlayerIdx);

		m_GameManager->GetPlayerProperties(CurrentPlayerIdx, Name, Score, Color);
		m_GameManager->StartPlayerTurn(CurrentPlayer, false);
		m_GameManager->GetUIManager()->SetCurrentPlayerName(Name.c_str(), Color.r, Color.g, Color.b);
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->DiasbleLayoutPositioning(true);
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->SetLetterVisibility(CurrentPlayer->GetUsedLetters());
		m_GameManager->GetUIManager()->GetPlayerLetters(CurrentPlayerIdx)->SetVisible(true);
	}

#ifndef PLATFORM_ANDROID
	#undef size_t
	#undef wchar_t
#endif

	StateFile.close();
}

void CGameState::LoadGameState()
{
	std::string FilePath = m_GameManager->GetWorkingDir() + 
#ifdef PLATFORM_ANDROID
		"/state.dat";
#else
		"state.dat";
#endif
	std::ifstream StateFile(FilePath, std::ifstream::binary);

	if (StateFile.fail())
		return;

	int GameState;
	int PlayerCountIdx;
	int BoardSize;
	int TimeLimit;
	int Difficulty;
	bool ComputerOpponentEnabled;

	StateFile.read((char *)&GameState, sizeof(int));
	StateFile.read((char *)&PlayerCountIdx, sizeof(int));
	StateFile.read((char *)&BoardSize, sizeof(int));
	StateFile.read((char *)&TimeLimit, sizeof(int));
	StateFile.read((char *)&ComputerOpponentEnabled, sizeof(bool));
	StateFile.read((char *)&Difficulty, sizeof(int));

	m_FilePos = StateFile.tellg();

    if (StateFile.fail())
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

void CGameState::SaveBoardState(const CGameBoard& currBoard, const CGameBoard& prevBoard)
{
	m_CurrBoardState = new CGameBoard;
	m_PrevBoardState = new CGameBoard;
	*m_CurrBoardState = currBoard;
	*m_PrevBoardState = prevBoard;
}

void CGameState::SavePlayerCount(int playerCount, bool computerOn)
{
	m_PlayerLetters.reserve(playerCount + static_cast<int>(computerOn));
	m_ComputerOn = computerOn;
}

void CGameState::SavePlayerLetters(size_t idx, const std::vector<std::wstring>& letters)
{
	if (idx >= m_PlayerLetters.size())
		return;

	m_PlayerLetters[idx] = letters;
}

void CGameState::SavePlayerSteps(const std::vector<TPlayerStep>& playerSteps)
{
	m_PlayerSteps = playerSteps;
}

void CGameState::SaveLetterPoolState(const std::map<wchar_t, int>& letterPool)
{
	m_LettersInPool = letterPool;
}

void CGameState::SaveCameraState(float tiltAngle, float rotAngle)
{
	m_CameraTiltAngle = tiltAngle;
	m_CameraRotAngle = rotAngle;
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





