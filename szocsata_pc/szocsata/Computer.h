#pragma once

#include "Player.h"
#include "WordTree.h"
#include "Structs.h"

struct TField;

class CComputer : public CPlayer
{
public:

	CComputer(CGameManager* gameManager);

	TComputerStep BestWord(int idx) { return m_BestWords[idx]; }
	TComputerStep GetComputerStep() { return m_ComputerStep; }
	void SetComputerStep(size_t idx) { m_ComputerStep = m_BestWords[idx];}//TComputerStep(m_BestWords[idx].m_Word.m_Word, m_BestWords[idx].m_Word.m_X, m_BestWords[idx].m_Word.m_Y, m_BestWords[idx].m_Word.m_Horizontal, m_BestWords[idx].m_Score, m_BestWords[idx].m_UsedLetters, m_BestWords[idx].m_CrossingWords, true); }
	int BestWordCount() { return m_BestWords.size(); }
	void ResetComputerStep() {m_ComputerStep.m_Word.m_Word = nullptr;}
 
	void BackTrackWord(CWordTree::TNode* node, std::wstring& word, int x, int y, bool horizontal, CBinaryBoolList usedLetters);
	void GetWordsInFieldList(const std::wstring& letters, CWordTree::TNode* node, CBinaryBoolList usedLetters, std::vector<TField*>& fieldList, int wordStartIdx, int pos, bool horizontal, int charCount = 1);
	void ComputeRowCol(int idx, bool rows);
	void CalculateStep();
	bool FindWord(const std::wstring& word, int score, bool horizontal = false, bool checkAlingnment = false, size_t** idx = nullptr);
	void AddResult(const TWordPos& word, CBinaryBoolList usedLetters);
	void SaveComputerStep(std::ofstream& fileStream);
	void LoadComputerStep(std::ifstream& fileStream);

	virtual bool IsComputer() override { return true; }

private:

	std::vector<TComputerStep> m_BestWords;
	TComputerStep m_ComputerStep;
};