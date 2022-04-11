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
	std::string FilePath = m_GameManager->GetWorkingDir() + "/state.dat";
	std::ofstream StateFile(FilePath, std::ofstream::binary);

	if (StateFile.fail())
		return;

	int GameState = m_GameManager->GetGameState();
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

	if (m_GameManager->GameScreenActive(static_cast<CGameManager::EGameState>(GameState)))
	{
		int TileCount;
		CConfig::GetConfig("tile_count", TileCount);

		for (int x = 0; x < TileCount; ++x)
		{
			for (int y = 0; y < TileCount; ++y)
			{
				wchar_t chr = m_GameManager->Board(x, y).m_Char;
				int height = m_GameManager->Board(x, y).m_Height;

				StateFile.write((char *)&chr, sizeof(wchar_t));
				StateFile.write((char *)&height, sizeof(int));
			}
		}

        int PlayerCount = PlayerCountIdx + 1 + (ComputerOpponentEnabled ? 1 : 0);

        for (size_t i = 0; i < PlayerCount; ++i)
		{
			std::wstring Letters = m_GameManager->GetPlayerLetters(i);
			int LetterCount = Letters.length();
			StateFile.write((char *)&LetterCount, sizeof(int));

			for (size_t j = 0; j < LetterCount; ++j)
				StateFile.write((char *)&Letters.at(j), sizeof(wchar_t));
		}
	}

	StateFile.close();
}

void CGameState::LoadPlayerAndBoardState()
{
	std::string FilePath = m_GameManager->GetWorkingDir() + "/state.dat";
	std::ifstream StateFile(FilePath, std::ifstream::binary);
	StateFile.seekg(m_FilePos);

	if (StateFile.fail())
		return;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	m_GameManager->SetBoardSize();

	float LetterHeight;
	float BoardHeight;

	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("letter_height", LetterHeight);

	int PlayerCountIdx = m_GameManager->GetUIManager()->GetPlayerCount();
	bool ComputerOpponentEnabled = m_GameManager->GetUIManager()->ComputerOpponentEnabled();

	for (int x = 0; x < TileCount; ++x) {
		for (int y = 0; y < TileCount; ++y) {
			wchar_t chr;
			int height;

			StateFile.read((char *)&chr, sizeof(wchar_t));
			StateFile.read((char *)&height, sizeof(int));

			m_GameManager->Board(x, y).m_Char = chr;
			m_GameManager->Board(x, y).m_Height = height;

			for (int h = 0; h < height; ++h)
				m_GameManager->AddLetterToBoard(x, TileCount - y - 1, chr, BoardHeight / 2.f + h * LetterHeight + LetterHeight / 2.f);
		}
	}

	int PlayerCount = PlayerCountIdx + 1 + (ComputerOpponentEnabled ? 1 : 0);
	for (size_t i = 0; i < PlayerCount; ++i)
	{
		int LetterCount;
		StateFile.read((char *)&LetterCount, sizeof(int));
		std::wstring Letters(LetterCount, L' ');

		for (size_t j = 0; j < LetterCount; ++j)
			StateFile.read((char *)&Letters.at(j), sizeof(wchar_t));

		m_GameManager->SetPlayerLetters(i, Letters);
		m_GameManager->GetUIManager()->GetPlayerLetters(i)->AddUILetters(LetterCount);
		m_GameManager->GetUIManager()->GetPlayerLetters(i)->SetVisible(false);
	}

	m_GameManager->StartPlayerTurn(m_GameManager->GetPlayer(0));
	m_GameManager->GetUIManager()->GetPlayerLetters(0)->SetVisible(true);
	m_GameManager->GetUIManager()->GetPlayerLetters(0)->DiasbleLayoutPositioning(true);

	((CUILayout*)(m_GameManager->GetUIManager()->GetUIElement(L"ui_game_screen_sub_layout3")))->SetBoxSizeProps(0, m_GameManager->GetUIManager()->GetScorePanelSize().x, m_GameManager->GetUIManager()->GetScorePanelSize().y, false);

	m_GameManager->GetUIManager()->GetPlayerLetters(0)->DiasbleLayoutPositioning(false);
}

void CGameState::LoadGameState()
{
	std::string FilePath = m_GameManager->GetWorkingDir() + "/state.dat";
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

    /*
    if (!m_GameManager->GameScreenActive(static_cast<CGameManager::EGameState>(GameState)))
		m_GameManager->SetGameState(GameState);
    else
		m_GameManager->m_StartOnGameScreen = true;
    */
	m_GameManager->GetUIManager()->SetPlayerCount(PlayerCountIdx);
	m_GameManager->GetUIManager()->SetBoardSize(BoardSize);
	m_GameManager->GetUIManager()->SetTimeLimitIdx(TimeLimit);
	m_GameManager->GetUIManager()->SetComputerOpponentEnabled(ComputerOpponentEnabled);
	m_GameManager->GetUIManager()->SetDifficulty(Difficulty);
	m_GameManager->SetTileCount();
	m_GameManager->SetBoardSize();
	m_GameManager->m_SavedGameState = static_cast<CGameManager::EGameState>(GameState);

	/*
	if (GameState != CGameManager::OnStartGameScreen && GameState != CGameManager::OnStartScreen && GameState != CGameManager::OnRankingsScreen)
		m_GameManager->FinishRenderInit();
*/

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






