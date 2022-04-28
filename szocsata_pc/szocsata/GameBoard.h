#pragma once

#include "Structs.h"

#include <vector>


class CGameBoard
{
public:
	CGameBoard() {}
	CGameBoard(int size);

	void AddWord(TWordPos& word);
	void SetSize(int size);
	void Reset();

	int Size()
	{
		return mFields.size();
	}

	TField& operator()(int x, int y)
	{
		return mFields[x][y];
	}

private:

	std::vector<std::vector<TField>> mFields;
};
