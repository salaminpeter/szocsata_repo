#include "stdafx.h"
#include "GameBoard.h"



CGameBoard::CGameBoard(int size)
{
	SetSize(size);
}

void CGameBoard::SetSize(int size)
{
	for (int i = 0; i < size; ++i)
	{
		mFields.push_back(std::vector<TField>());

		for (int j = 0; j < size; ++j)
		{
			mFields[i].push_back(TField());
		}
	}
}

void CGameBoard::Reset()
{
	for (size_t i = 0; i < mFields.size(); ++i)
		mFields[i].clear();

	mFields.clear();
}


void CGameBoard::AddWord(TWordPos& word)
{
	if (word.m_Horizontal)
	{
		for (int i = word.m_X; i < word.m_X + word.m_Word->length(); ++i)
		{
			if (word.m_Word->at(i - word.m_X) == mFields[i][word.m_Y].m_Char)
				continue;

			mFields[i][word.m_Y].m_Char = word.m_Word->at(i - word.m_X);
			mFields[i][word.m_Y].m_Height++;
		}
	}
	else
	{
		for (int i = word.m_Y; i < word.m_Y + word.m_Word->length(); ++i)
		{
			if (word.m_Word->at(i - word.m_Y) == mFields[word.m_X][i].m_Char)
				continue;

			mFields[word.m_X][i].m_Char = word.m_Word->at(i - word.m_Y);
			mFields[word.m_X][i].m_Height++;
		}
	}
}


