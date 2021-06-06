#include "stdafx.h"
#include "Player.h"
#include "GameManager.h"
#include "Config.h"

#include <sstream>
#include <algorithm>

int CPlayer::m_PlayerCount = 0;

CPlayer::CPlayer(CGameManager* gameManager) : m_GameManager(gameManager)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	m_Letters.resize(LetterCount, L' ');  
	std::wstringstream ss;
	ss << L"jatekos " << ++m_PlayerCount;
	m_Name = ss.str();
}

void CPlayer::SetLetterUsed(int letterIdx, bool used)
{
	m_UsedLetters.SetFlag(letterIdx, used);

	if (used)
		m_Letters[letterIdx] = L' ';
}

bool CPlayer::IsLetterUsed(int letterIdx)
{
	return m_UsedLetters.GetFlag(letterIdx);
}


void CPlayer::AddLetters(const std::vector<wchar_t>& letters)
{
	int LetterIdx = 0;

	for (size_t i = 0; i < m_Letters.length() && LetterIdx < letters.size(); ++i)
	{
		if (m_Letters[i] == L' ')
			m_Letters[i] = letters[LetterIdx++];
	}
}

void CPlayer::ArrangeLetters()
{
	for (size_t i = 0; i < m_Letters.length(); ++i)
	{
		if (m_Letters[i] == L' ')
		{
			for (size_t j = i + 1; j < m_Letters.length(); ++j)
			{
				if (m_Letters[j] != L' ')
				{
					m_Letters[i] = m_Letters[j];
					m_Letters[j] = L' ';
					break;
				}
			}
		}
	}

	while (m_Letters.back() == L' ')
		m_Letters.pop_back();
}

int CPlayer::GetUsedLetterCount()
{
	int Count = 0;

	for (int i = 0; i < m_Letters.size(); ++i)
	{
		if (m_UsedLetters.GetFlag(i))
			Count++;
	}
	
	return Count;	
}

int CPlayer::RemoveLetter(wchar_t c)
{
	for (size_t i = 0; i < m_Letters.size(); ++i)
	{
		if (m_Letters[i] == c)
		{
			m_Letters[i] = L' ';
			return i;
		}
	}

	return -1;
}

void CPlayer::SetLetter(int letterIdx, wchar_t c)
{
	m_Letters[letterIdx] = c;
}


int CPlayer::GetUsedLetterIdx(wchar_t c)
{
	for (size_t i = 0; i < m_Letters.length(); ++i)
	{
		if (m_Letters[i] == c && m_UsedLetters.GetFlag(i))
			return i;
	}

	return -1;
}
