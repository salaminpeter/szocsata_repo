#pragma once

#include "BinaryBoolList.h"

#include <algorithm>
#include <vector>
#include <string>
#include <glm/glm.hpp>

class CGameManager;

class CPlayer
{
public:
	
	CPlayer(CGameManager* gameManager);
	~CPlayer() {--m_PlayerCount;}

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
	int GetLetterCount() {return std::count_if(m_Letters.begin(), m_Letters.end(), [](wchar_t c) {return c != L' ';});}
	void AddScore(int score) {m_Score += score;}
	void ResetUsedLetters() {m_UsedLetters.Reset();}
	void SetUsedLetters(CBinaryBoolList usedLetters) {m_UsedLetters = usedLetters;}
	void SetUsedLetters(unsigned int usedLetters) {m_UsedLetters.SetList(usedLetters);}
	bool LetterUsed(int i) {return m_UsedLetters.GetFlag(i);}
	int GetScore() {return m_Score;}
	std::wstring GetName() {return m_Name;}
	std::wstring&  GetLetters() {return m_Letters;}
	std::wstring&  GetAllLetters() {return m_AllLetters;}
	CBinaryBoolList GetUsedLetters() {return m_UsedLetters;}
	glm::vec3 GetColor() {return m_Color;}
	void SetAllLetters() {m_AllLetters = m_Letters;}
	void SetAllLetters(const wchar_t* letters) {m_AllLetters = letters;}

	virtual bool IsComputer() {return false;}

public:

	static int m_PlayerCount;

public:
	
	glm::vec3 m_Color;
	CGameManager* m_GameManager;
	std::wstring m_Letters;  //betuk, ahol a mar elhelyezettek helyett spaceek vannak
	std::wstring m_AllLetters; //a komplett betusor
	std::wstring m_Name;
	CBinaryBoolList m_UsedLetters;
	int m_Score = 0;
	bool m_Passed = false;
};