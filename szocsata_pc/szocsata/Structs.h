#pragma once

#include "BinaryBoolList.h"

#include <string>
#include <vector>

struct TField
{
	wchar_t m_Char;
	int m_Height;

	TField(wchar_t chr, int height) : m_Char(chr), m_Height(height) {}
	TField() : m_Char(L'*'), m_Height(0) {}
};

struct TWordPos
{
	std::wstring* m_Word = nullptr;
	int m_X, m_Y;
	int m_WordLength = 0;
	bool m_Horizontal;

	TWordPos() {}
	TWordPos(std::wstring* word, int x, int y, bool h, int length = 0) : m_Word(word), m_X(x), m_Y(y), m_Horizontal(h), m_WordLength(length) //TODO letterindeices move
	{
		m_WordLength = word ? word->length() : length;
	}
};

struct TComputerStep
{
	TComputerStep(std::wstring* word, int x, int y, bool h, int score, CBinaryBoolList usedLetters, std::vector<TWordPos>& crossingWords) : m_Word(word, x, y, h), m_Score(score), m_UsedLetters(usedLetters), m_CrossingWords(crossingWords)
	{
		m_CrossingWords.reserve(word->length());
	}

	TComputerStep() {}
	
	TWordPos m_Word;
	int m_Score = 0;
	CBinaryBoolList m_UsedLetters;
	std::vector<TWordPos> m_CrossingWords;
};

struct TPlayerStep
{
	wchar_t m_Char;
	int m_XPosition;
	int m_YPosition;
	int m_LetterIdx;

	TPlayerStep(wchar_t c, int x, int y, int letterIdx) : m_Char(c), m_XPosition(x), m_YPosition(y), m_LetterIdx(letterIdx) {}
};