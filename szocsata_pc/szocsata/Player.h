#pragma once

#include "BinaryBoolList.h"

#include <vector>
#include <string>
#include <glm/glm.hpp>

class CGameManager;

class CPlayer
{
public:
	
	CPlayer(CGameManager* gameManager);

	void AddLetters(const std::vector<wchar_t>& letters);
	void ArrangeLetters();
	void SetLetterUsed(int letterIdx, bool used);
	bool IsLetterUsed(int letterIdx);
	int GetUnUsedLetterCount();
	int GetUsedLetterCount();
	int GetUsedLetterIdx(wchar_t c);
	void SetLetter(int letterIdx, wchar_t c);
	void RemoveLetter(size_t idx);
	std::vector<size_t> GetLetterIndicesForWord(const std::wstring& word);

	void SetColor(float r, float g, float b) {m_Color = glm::vec3(r, g, b); }
	int GetLetterCount() {return m_Letters.size();}
	void AddScore(int score) {m_Score += score;}
	void ResetUsedLetters() {m_UsedLetters.Reset();}
	void SetUsedLetters(CBinaryBoolList usedLetters) {m_UsedLetters = usedLetters;}
	bool LetterUsed(int i) {return m_UsedLetters.GetFlag(i);}
	int GetScore() {return m_Score;}
	std::wstring GetName() {return m_Name;}
	std::wstring&  GetLetters() {return m_Letters;}
	CBinaryBoolList GetUsedLetters() {return m_UsedLetters;}
	glm::vec3 GetColor() {return m_Color;}

public:

	static int m_PlayerCount;

public:
	
	glm::vec3 m_Color;
	CGameManager* m_GameManager;
	std::wstring m_Letters;
	std::wstring m_Name;
	CBinaryBoolList m_UsedLetters;
	int m_Score = 0;
	bool m_Passed = false;
};