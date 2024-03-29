#include "stdafx.h"
#include "Computer.h"
#include "Structs.h"
#include "GameManager.h"
#include "Config.h"

#include <functional>
#include <algorithm>


CComputer::CComputer(CGameManager* gameManager) : CPlayer(gameManager)
{
	int BestWordCount;
	CConfig::GetConfig("best_word_count", BestWordCount);

	m_BestWords.reserve(BestWordCount);
	m_Name = L"computer";
}

void CComputer::SaveComputerStep(std::ofstream& fileStream)
{
	m_ComputerStep.m_Word.SaveToFile(fileStream);
	fileStream.write((char *)&m_ComputerStep.m_Score, sizeof(int));
	unsigned UsedLetters = m_ComputerStep.m_UsedLetters.GetList();
	fileStream.write((char *)&UsedLetters, sizeof(unsigned));
	size_t CrossingWordCount = m_ComputerStep.m_CrossingWords.size();
	fileStream.write((char *)&CrossingWordCount, sizeof(size_t));
	if (!fileStream.good())
		return;

	for (auto CrossingWord : m_ComputerStep.m_CrossingWords)
		CrossingWord.SaveToFile(fileStream);

	if (!fileStream.good())
		return;

}

void CComputer::LoadComputerStep(std::ifstream& fileStream)
{
	m_ComputerStep.m_Word.LoadFromFile(fileStream);
	fileStream.read((char *)&m_ComputerStep.m_Score, sizeof(int));
	unsigned UsedLetters;
	fileStream.read((char *)&UsedLetters, sizeof(unsigned));
	m_ComputerStep.m_UsedLetters.SetList(UsedLetters);
	size_t CrossingWordCount;
	fileStream.read((char *)&CrossingWordCount, sizeof(size_t));
	m_ComputerStep.m_CrossingWords.reserve(CrossingWordCount);

	for (size_t i = 0; i < CrossingWordCount; ++i)
	{
		m_ComputerStep.m_CrossingWords.push_back(TWordPos());
		m_ComputerStep.m_CrossingWords.back().LoadFromFile(fileStream);
	}
}


bool CComputer::FindWord(const std::wstring& word, int score, bool horizontal, bool checkAlingnment, size_t** idx)
{
	for (size_t i = 0; i < m_BestWords.size(); ++i)
	{
		if ((!checkAlingnment || horizontal == m_BestWords[i].m_Word.m_Horizontal) && word == *(m_BestWords[i].m_Word.m_Word) && score == m_BestWords[i].m_Score)
		{
			if (idx && *idx)
				**idx = i;
			return true;
		}
	}

	return false;
}

void CComputer::AddResult(const TWordPos& wordPos, CBinaryBoolList usedLetters)
{
	int BestWordCount;
	int TileCount;
	CConfig::GetConfig("best_word_count", BestWordCount);
	CConfig::GetConfig("tile_count", TileCount);

	std::vector<TWordPos> CrossingWords;

	int Score = m_GameManager->CalculateScore(wordPos, &CrossingWords);

	//az egyik crossingword ertelmetlen volt
	if (Score == 0)
		return;

	size_t* WordIdx = new size_t;

	//ha mar van ilyen szavunk, de ez a szo kozelebb van a tabla kozepehez akkor valasszuk ezt helyette	
	if (FindWord(*wordPos.m_Word, Score, wordPos.m_Horizontal, true, &WordIdx))
	{
		TWordPos& Word = m_BestWords[*WordIdx].m_Word;
		int WordLength = Word.m_Word->length() / 2;
		int CurrWordLength = wordPos.m_Word->length() / 2;

		if (std::labs(wordPos.m_X + CurrWordLength - TileCount / 2) + std::labs(wordPos.m_Y + CurrWordLength - TileCount / 2) < std::labs(Word.m_X + WordLength / 2 - TileCount / 2) + std::fabs(Word.m_Y + WordLength / 2 - TileCount / 2))
		{
			m_BestWords[*WordIdx].m_Word = wordPos;
			m_BestWords[*WordIdx].m_CrossingWords = CrossingWords;
			delete WordIdx;
			return;
		}
	}

	delete WordIdx;

	int Difficulty = m_GameManager->GetDifficulty();
	int MaxScore;
    int MaxScoreOneResult = 0;

    if (Difficulty == 0)
        MaxScore = 3;
	else if (Difficulty == 1)
    {
        MaxScore = 9;
        MaxScoreOneResult = 3;
    }
	else if (Difficulty == 2)
	{
        MaxScore = 20;
        MaxScoreOneResult = 5;
    }
	else if (Difficulty == 3)
		MaxScore = 1000;

	int WordsWithScore = std::count_if(m_BestWords.begin(), m_BestWords.end(), [&Score](const auto& compStep) {return compStep.m_Score == Score;});
	bool AddWordWithScore = (Score >= MaxScoreOneResult && Score <= MaxScore &&  WordsWithScore < 4) || (Score <= MaxScore && WordsWithScore == 0);
	bool AddWord = Score != 0 && AddWordWithScore && !FindWord(*wordPos.m_Word, Score); //TODO konstansok konfigba!

	if (AddWord)
	{
		m_BestWords.push_back(TComputerStep(wordPos.m_Word, wordPos.m_X, wordPos.m_Y, wordPos.m_Horizontal, Score, usedLetters, CrossingWords));
		std::sort(m_BestWords.begin(), m_BestWords.end(), [](TComputerStep& a, TComputerStep& b) {return a.m_Score > b.m_Score;}); //TODO biztos kell a sort?
	}
}


void CComputer::BackTrackWord(CWordFlatTree::TNode* node, std::wstring& word, int x, int y, bool horizontal, CBinaryBoolList usedLetters)
{
	if (!node) {

		for (int i = 0; i < word.length(); ++i)
		{
			int xp = horizontal ? x + i : x;
			int yp = horizontal ? y : y + i;

			TField f = m_GameManager->Board(xp, yp);
			wchar_t cc = word.at(i);

			if (m_GameManager->Board(xp, yp).m_Char != word.at(i))
			{
				AddResult(TWordPos(&word, x, y, horizontal), usedLetters);
				return;
			}
		}

		return;
	}

	word = node->m_Char + word;
	BackTrackWord(node->Parent(), word, x, y, horizontal, usedLetters);
}


void CComputer::GetWordsInFieldList(const std::wstring& letters, CWordFlatTree::TNode* node, CBinaryBoolList usedLetters, std::vector<TField*>& fieldList, int wordStartIdx, int pos, bool horizontal, int charCount)
{
	if (fieldList.size() == wordStartIdx + charCount)
		return;

	CWordFlatTree::TNode* ChildNode = nullptr;
	wchar_t CurrentLetter = fieldList[wordStartIdx + charCount - 1]->m_Char;
	int CurrentFieldHeight = fieldList[wordStartIdx + charCount - 1]->m_Height;

	//vizsgalando szo elott van egy betu a tablan akkor ezt a fazist ugorhatjuk mert mar az osszes lehetseges szo ellenorizve lett
	if (wordStartIdx != 0 && !node && fieldList[wordStartIdx - 1]->m_Char != L'*')
		return;

	if (CurrentLetter != L'*')
	{
		if (!node)
			GetWordsInFieldList(letters, m_GameManager->WordTreeRoot(CurrentLetter), CBinaryBoolList(), fieldList, wordStartIdx, pos, horizontal, charCount + 1);
		else
		{
			ChildNode = node->FindChild(CurrentLetter);

			if (ChildNode) {
				if (ChildNode->m_WordEnd && (fieldList.size() == wordStartIdx + charCount || fieldList[wordStartIdx + charCount]->m_Char == L'*'))
				{
					int WordX = horizontal ? wordStartIdx : pos;
					int WordY = horizontal ? pos : wordStartIdx;
					BackTrackWord(ChildNode, *new std::wstring(), WordX, WordY, horizontal, usedLetters);
				}

				GetWordsInFieldList(letters, ChildNode, usedLetters, fieldList, wordStartIdx, pos, horizontal, charCount + 1);
			}
		}

		if (CurrentFieldHeight == 5)
			return;
	}

	for (unsigned int i = 0; i < letters.length(); ++i)
	{
		if (usedLetters.GetFlag(i) || letters[i] == CurrentLetter)
			continue;

		if (!node)
		{
			CBinaryBoolList UsedLetters = usedLetters;
			UsedLetters.SetFlag(i, true);
			CWordFlatTree::TNode* RootNode = m_GameManager->WordTreeRoot(letters[i]);

			if (!RootNode)
				return;

			GetWordsInFieldList(letters, RootNode, UsedLetters, fieldList, wordStartIdx, pos, horizontal, charCount + 1);
		}

		else if (ChildNode = node->FindChild(letters[i]))
		{
			if (ChildNode->m_WordEnd && (fieldList.size() == wordStartIdx + charCount || fieldList[wordStartIdx + charCount]->m_Char == L'*'))
			{
				int WordX = horizontal ? wordStartIdx : pos;
				int WordY = horizontal ? pos : wordStartIdx;
				CBinaryBoolList UsedLetters = usedLetters;
				UsedLetters.SetFlag(i, true);
				BackTrackWord(ChildNode, *new std::wstring(), WordX, WordY, horizontal, UsedLetters);
			}

			CBinaryBoolList UsedLetters = usedLetters;
			UsedLetters.SetFlag(i, true);
			GetWordsInFieldList(letters, ChildNode, UsedLetters, fieldList, wordStartIdx, pos, horizontal, charCount + 1);
		}
	}
}

void CComputer::ComputeRowCol(int idx, bool rows)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	if (rows)
	{
		std::vector<TField*> Row;

		for (int i = 0; i < TileCount; ++i)
			Row.push_back(&m_GameManager->Board(i, idx));

		CWordFlatTree::TNode* CurrentNode = nullptr;

		for (int i = 0; i < Row.size(); ++i)
		{
			if (i != 0 && Row[i - 1]->m_Char == L'*')
				CurrentNode = nullptr;

			else if (i != 0 && Row[i - 1]->m_Char != L'*' && !CurrentNode)
				CurrentNode = m_GameManager->WordTreeRoot(Row[i - 1]->m_Char);

			if (Row[i]->m_Char != L'*')
			{
				if (!CurrentNode)
				{
					GetWordsInFieldList(m_Letters, nullptr, CBinaryBoolList(), Row, i, idx, true);
				}
				else
				{
					GetWordsInFieldList(m_Letters, CurrentNode, CBinaryBoolList(), Row, i - CurrentNode->m_Level - 1, idx, true, CurrentNode->m_Level + 2);
					CurrentNode = CurrentNode->FindChild(Row[i]->m_Char);
				}
			}
			else {
				if (!CurrentNode)
					GetWordsInFieldList(m_Letters, nullptr, CBinaryBoolList(), Row, i, idx, true);
				else
					GetWordsInFieldList(m_Letters, CurrentNode, CBinaryBoolList(), Row, i - CurrentNode->m_Level - 1, idx, true, CurrentNode->m_Level + 2);
			}
		}
	}
	else
	{
		std::vector<TField*> Column;

		for (int i = 0; i < TileCount; ++i)
			Column.push_back(&m_GameManager->Board(idx, i));

		CWordFlatTree::TNode* CurrentNode = nullptr;

		for (int i = 0; i < Column.size(); ++i)
		{
			if (i != 0 && Column[i - 1]->m_Char == L'*')
				CurrentNode = nullptr;

			else if (i != 0 && Column[i - 1]->m_Char != L'*' && !CurrentNode)
				CurrentNode = m_GameManager->WordTreeRoot(Column[i - 1]->m_Char);

			if (Column[i]->m_Char != L'*')
			{
				if (!CurrentNode)
				{
					GetWordsInFieldList(m_Letters, nullptr, CBinaryBoolList(), Column, i, idx, false);
				}
				else
				{
					GetWordsInFieldList(m_Letters, CurrentNode, CBinaryBoolList(), Column, i - CurrentNode->m_Level - 1, idx, false, CurrentNode->m_Level + 2);
					CurrentNode = CurrentNode->FindChild(Column[i]->m_Char);
				}
			}
			else {
				if (!CurrentNode)
					GetWordsInFieldList(m_Letters, nullptr, CBinaryBoolList(), Column, i, idx, false);
				else
					GetWordsInFieldList(m_Letters, CurrentNode, CBinaryBoolList(), Column, i - CurrentNode->m_Level - 1, idx, false, CurrentNode->m_Level + 2);
			}
		}
	}
}

void CComputer::CalculateStep()
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	m_BestWords.clear();
	m_UsedLetters.Reset();

	int Center = TileCount / 2;

	for (int i = 0; i < TileCount; ++i)
	{
		ComputeRowCol(i, true);
		ComputeRowCol(i, false);
	}
}
