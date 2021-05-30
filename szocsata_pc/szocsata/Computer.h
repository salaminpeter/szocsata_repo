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
	int BestWordCount() { return m_BestWords.size(); }

	void BackTrackWord(CWordTree::TNode* node, std::wstring& word, int x, int y, bool horizontal, CBinaryBoolList usedLetters);
	void GetWordsInFieldList(const std::wstring& letters, CWordTree::TNode* node, CBinaryBoolList usedLetters, std::vector<TField*>& fieldList, int wordStartIdx, int pos, bool horizontal, int charCount = 0);
	void ComputeRowCol(int idx, bool rows, std::vector<std::wstring*>& words);
	void CalculateStep();
	bool FindWord(const std::wstring& word, int score, bool horizontal = false, bool checkAlingnment = false, size_t** idx = nullptr);
	void AddResult(const TWordPos& word, CBinaryBoolList usedLetters);

private:

	std::vector<TComputerStep> m_BestWords;
};