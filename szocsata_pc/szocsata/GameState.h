#pragma once

#include <map>
#include "Structs.h"

class CGameBoard;
class CGameManager;

class CGameState
{
public:
	
	CGameState(CGameManager* gameManager) : m_GameManager(gameManager) {}

	void SaveGameState();
	void LoadGameState();
	void LoadPlayerAndBoardState();

	void SaveBoardState(const CGameBoard& currBoard, const CGameBoard& prevBoard);
	void SavePlayerCount(int playerCount, bool computerOn);
	void SavePlayerLetters(size_t idx, const std::vector<std::wstring>& letters);
	void SavePlayerSteps(const std::vector<TPlayerStep>& playerSteps);
	void SaveLetterPoolState(const std::map<wchar_t, int>& letterPool);
	void SaveCameraState(float tiltAngle, float RotAngle);
	void RemoveSaveFile();

private:
	
	int m_PlayerCount;
	bool m_ComputerOn;
	int m_CurrentPlayerIdx;
	int m_FilePos;

	std::vector<std::vector<std::wstring>> m_PlayerLetters;
	std::vector<TPlayerStep> m_PlayerSteps;

	CGameBoard* m_CurrBoardState;
	CGameBoard* m_PrevBoardState;

	std::map<wchar_t, int> m_LettersInPool;

	float m_CameraTiltAngle;
	float m_CameraRotAngle;

	int m_GameState;

	CGameManager* m_GameManager;
};
