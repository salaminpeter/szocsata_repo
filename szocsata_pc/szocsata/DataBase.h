#pragma once

#include <map>
#include <vector>

#include "WordTree.h"

class CWordTree;

class CDataBase
{
public:

	int linecount = 0;
	void LoadDataBase(const char* dbFilePath);
    bool WordExists(std::wstring& word, CWordFlatTree::TNode* node = nullptr, int charIdx = 0);
	CWordFlatTree::TNode* GetWordTreeRoot(wchar_t c);
	void SaveDataBase(const char*path);

private:

//	std::map<wchar_t, CWordTree*> m_WordTrees;
	std::map<wchar_t, std::shared_ptr<CWordFlatTree>> m_WordTree;
};