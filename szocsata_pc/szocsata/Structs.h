#pragma once

#include "BinaryBoolList.h"

#include <string>
#include <vector>
#include<iostream>
#include<fstream>

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
	int m_WordLength = 0; //nem minden esetben kell magat a szot is tarolnunk, van amikor eleg csak a hossza (crossingwords)
	bool m_Horizontal;

	TWordPos(std::wstring* word, int x, int y, bool h, int length = 0, bool saveWord = false) : m_Word(word), m_X(x), m_Y(y), m_Horizontal(h) //TODO letterindeices move
	{
		m_WordLength = word ? word->length() : length;

		if (saveWord)
			m_Word = new std::wstring(*word);
	}

	TWordPos() = default;

	void SaveToFile(std::ofstream& fileStream)
	{
		int WordLength = GetWordLength();
		bool OnlyWordLengthSaved = m_Word == nullptr;
		const wchar_t* WordStr = m_Word ? m_Word->c_str() : nullptr;
		fileStream.write((char *)&OnlyWordLengthSaved, sizeof(bool));
		fileStream.write((char *)&WordLength, sizeof(int));
		
		if (!OnlyWordLengthSaved)
			fileStream.write((char *)&WordStr, WordLength * sizeof(wchar_t));
		
		fileStream.write((char *)&m_X, sizeof(int));
		fileStream.write((char *)&m_Y, sizeof(int));
		fileStream.write((char *)&m_Horizontal, sizeof(bool));
	}

	void LoadFromFile(std::ifstream& fileStream)
	{
		bool OnlyWordLengthSaved;
		fileStream.read((char *)&OnlyWordLengthSaved, sizeof(bool));
		fileStream.read((char *)&m_WordLength, sizeof(int));

		if (!OnlyWordLengthSaved)
		{
			std::vector<wchar_t> Word(m_WordLength);
			fileStream.read((char *)&(Word[0]), m_WordLength * sizeof(wchar_t));
			m_Word = new std::wstring(&Word[0]); //TODO!!!! lewg6en shared pointer mert betoltes eseteben nem egy elore kiszamitott word halmazbol valasztunk hanem betoltunk egyet, es ebben az esetben itt kell majd torolni
		}

		fileStream.read((char *)&m_X, sizeof(int));
		fileStream.read((char *)&m_Y, sizeof(int));
		fileStream.read((char *)&m_Horizontal, sizeof(bool));
	}

	size_t GetWordLength() const {return m_Word ? m_Word->length() : m_WordLength; } 
};	



struct TComputerStep
{
	TComputerStep(std::wstring* word, int x, int y, bool h, int score, CBinaryBoolList usedLetters, std::vector<TWordPos>& crossingWords, bool saveWord = false) : m_Word(word, x, y, h, word->length(), saveWord), m_Score(score), m_UsedLetters(usedLetters), m_CrossingWords(crossingWords)
	{
		m_CrossingWords.reserve(word->length());
	}

	TComputerStep() = default;

	bool IsStepValid() {return m_Word.GetWordLength() != 0; }
	
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