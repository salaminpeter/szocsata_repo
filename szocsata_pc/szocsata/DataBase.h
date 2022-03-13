#pragma once

#include <map>

#include "WordTree.h"

class CWordTree;

class CDataBase
{
public:
	int linecount = 0;
	void LoadDataBase(const char* dbFilePath);
	bool WordExists(std::wstring& word, CWordTree::TNode* node = nullptr, int charIdx = 0);
	
	CWordTree::TNode* GetWordTreeRoot(wchar_t c)
	{
		if (m_WordTrees.find(c) == m_WordTrees.end())
			return nullptr;

		return m_WordTrees[c]->Root();
	}
		
private:
	std::map<wchar_t, CWordTree*> m_WordTrees;
};