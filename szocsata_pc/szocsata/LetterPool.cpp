#include "stdafx.h"
#include "LetterPool.h"
#include "Config.h"

#include <string>
#include <ctime>
#include <algorithm>


CLetterPool::CLetterPool()
{
	std::srand(std::time(nullptr));

	m_LettersForSize[7] = 6;
	m_LettersForSize[8] = 7;
	m_LettersForSize[9] = 9;
	m_LettersForSize[10] = 10;
	m_LettersForSize[11] = 10;
	m_LettersForSize[12] = 10;

}


int CLetterPool::DealLetters(std::wstring& letters)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	int AddedLetterCount = 0;
	int InitialLetterCount = std::count_if(letters.begin(), letters.end(), [](wchar_t c) {return c != L' ';});
	int RemainingLetters = GetRemainingLetterCount();

	if (RemainingLetters == 0)
		return 0;

	int Count = (LetterCount - InitialLetterCount < RemainingLetters ? LetterCount - InitialLetterCount : RemainingLetters);

	while (m_LetterIdx.size() != 0)
	{
		size_t Idx = letters.find_first_of(L' ');

		if (Idx == std::wstring::npos)
			return AddedLetterCount;

		int RandVal = std::rand() / ((RAND_MAX + 1u) / (m_LetterIdx.size()));

		if (RandVal == m_LetterIdx.size())  //TODO
			RandVal--;

		if (m_Letters[m_LetterIdx[RandVal]] > 0)
		{
			AddedLetterCount++;
			letters[Idx] = m_LetterIdx[RandVal];
			m_Letters[m_LetterIdx[RandVal]]--;
			RemainingLetters--;

			if (m_Letters[m_LetterIdx[RandVal]] == 0)
			{
				m_LetterIdx[RandVal] = m_LetterIdx.back();
				m_LetterIdx.pop_back();

				if (m_LetterIdx.size() == 0)
					break;
			}

			if (RemainingLetters == 0 || AddedLetterCount == Count)
				return AddedLetterCount;
		}
	}

	return AddedLetterCount;
}

void CLetterPool::ClearEmptyLetterIndices()
{
	size_t i = 0;
	while (m_LetterIdx.size() > i)
	{
		if (m_Letters.find(m_LetterIdx[i]) == m_Letters.end() || m_Letters[m_LetterIdx[i]] == 0)
		{
			m_LetterIdx[i] = m_LetterIdx.back();
			m_LetterIdx.pop_back();
		}
		else
			++i;
	}
}

int CLetterPool::GetRemainingLetterCount()
{
	int Count = 0;

	for (auto it = m_Letters.begin(); it != m_Letters.end(); ++it)
		Count += it->second;

	return Count;
}

int CLetterPool::GetLetterCount(int idx)
{
    if (idx >= m_LetterIdx.size())
        return -1;

    return m_Letters[m_LetterIdx[idx]];
}

bool CLetterPool::SetLetterCount(int idx, int count)
{
    if (idx >= m_LetterIdx.size())
        return false;

    m_Letters[m_LetterIdx[idx]] = count;
    return true;
}

void CLetterPool::Init(bool initLettersCount)
{
	m_LetterIdx.resize(32);

	m_LetterIdx[0] = L'a';
	m_LetterIdx[1] = L'á';
	m_LetterIdx[2] = L'b';
	m_LetterIdx[3] = L'c';
	m_LetterIdx[4] = L'd';
	m_LetterIdx[5] = L'e';
	m_LetterIdx[6] = L'é';
	m_LetterIdx[7] = L'f';
	m_LetterIdx[8] = L'g';
	m_LetterIdx[9] = L'h';
	m_LetterIdx[10] = L'i';
	m_LetterIdx[11] = L'í';
	m_LetterIdx[12] = L'j';
	m_LetterIdx[13] = L'k';
	m_LetterIdx[14] = L'l';
	m_LetterIdx[15] = L'm';
	m_LetterIdx[16] = L'n';
	m_LetterIdx[17] = L'o';
	m_LetterIdx[18] = L'ó';
	m_LetterIdx[19] = L'ö';
	m_LetterIdx[20] = L'ő';
	m_LetterIdx[21] = L'p';
	m_LetterIdx[22] = L'r';
	m_LetterIdx[23] = L's';
	m_LetterIdx[24] = L't';
	m_LetterIdx[25] = L'v';
	m_LetterIdx[26] = L'u';
	m_LetterIdx[27] = L'ú';
	m_LetterIdx[28] = L'ü';
	m_LetterIdx[29] = L'ű';
	m_LetterIdx[30] = L'z';
	m_LetterIdx[31] = L'y';

	if (initLettersCount) {
		m_Letters[L'a'] = 8;
		m_Letters[L'á'] = 5;
		m_Letters[L'b'] = 4;
		m_Letters[L'c'] = 3;
		m_Letters[L'd'] = 4;
		m_Letters[L'e'] = 10;
		m_Letters[L'é'] = 6;
		m_Letters[L'f'] = 3;
		m_Letters[L'g'] = 4;
		m_Letters[L'h'] = 3;
		m_Letters[L'i'] = 6;
		m_Letters[L'í'] = 3;
		m_Letters[L'j'] = 3;
		m_Letters[L'k'] = 6;
		m_Letters[L'l'] = 7;
		m_Letters[L'm'] = 6;
		m_Letters[L'n'] = 7;
		m_Letters[L'o'] = 4;
		m_Letters[L'ó'] = 5;
		m_Letters[L'ö'] = 4;
		m_Letters[L'ő'] = 4;
		m_Letters[L'p'] = 3;
		m_Letters[L'r'] = 4;
		m_Letters[L's'] = 7;
		m_Letters[L't'] = 7;
		m_Letters[L'v'] = 4;
		m_Letters[L'u'] = 3;
		m_Letters[L'ú'] = 3;
		m_Letters[L'ü'] = 4;
		m_Letters[L'ű'] = 4;
		m_Letters[L'z'] = 4;
		m_Letters[L'y'] = 5;
	}

	//set letter count based on board dimensions
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);
	float div = static_cast<float>(TileCount * TileCount) / 144.f;

	for (auto it = m_Letters.begin(); it != m_Letters.end(); ++it)
	{
		float Count = static_cast<float>(it->second) * div;
		float fp = Count - static_cast<int>(Count);

		if (fp < 0.5f)
			Count = static_cast<int>(Count);
		else
			Count = static_cast<int>(Count + 1.f);

		it->second = Count;
	}

	CConfig::AddConfig("letter_count", m_LettersForSize[TileCount]);
}
