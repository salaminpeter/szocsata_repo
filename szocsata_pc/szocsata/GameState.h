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
	void SaveComputerStep(std::ofstream& fileStream);
	void LoadComputerStep(std::ifstream& fileStream);
	void SaveCameraState(float tiltAngle, float RotAngle);
	void RemoveSaveFile();

private:
	
	int m_FilePos;
	CGameManager* m_GameManager;
};
