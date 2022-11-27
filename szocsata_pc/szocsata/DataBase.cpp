#include "stdafx.h"
#include "DataBase.h"
#include "WordTree.h"
#include "IOManager.h"


#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include "Config.h"
#include "FileHandler.h"



void CDataBase::LoadDataBase(const char* dbFilePath)
{
	std::wstringstream StrStream;
	std::wstring Str;

	if (!CIOManager::GetStreamStrForFile(dbFilePath, StrStream))
		return;

	while (std::getline(StrStream, Str))
	{
        CWordTree* WordTree;

		Str.erase(std::remove(Str.begin(), Str.end(), '\r'), Str.end());
		Str.erase(std::remove(Str.begin(), Str.end(), '\n'), Str.end());

		if (Str == L"")
			continue;

		if (m_WordTrees.find(Str[0]) != m_WordTrees.end())
			WordTree = m_WordTrees[Str[0]];
		else
		{
			m_WordTrees[Str[0]] = new CWordTree(Str[0]);
			WordTree = m_WordTrees[Str[0]];
		}

		WordTree->AddWord(Str, WordTree->Root());
		linecount++;
	}
}


bool CDataBase::WordExists(std::wstring& word, CWordTree::TNode* node, int charIdx)
{
	if (charIdx == word.length())
		return node->m_WordEnd;

	if (node)
	{
		CWordTree::TNode* ChildNode = node->FindChild(word.at(charIdx));

		if (!ChildNode)
			return false;

		return WordExists(word, ChildNode, ++charIdx);
	}
	else
	{
		if (m_WordTrees.find(word.at(0)) == m_WordTrees.end())
			return false;

		return WordExists(word, m_WordTrees[word.at(0)]->Root(), 1);
	}
}

