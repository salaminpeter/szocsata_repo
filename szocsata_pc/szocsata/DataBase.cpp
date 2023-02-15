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


CWordFlatTree::TNode* CDataBase::GetWordTreeRoot(wchar_t c)
{
	if (m_WordTree.find(c) == m_WordTree.end())
		return nullptr;

	return m_WordTree[c]->GetNode(0);
}

void CDataBase::WriteNodeToBinary(CWordTree::TNode* node, std::ofstream& stream)
{
	size_t ChildCount = node->m_Children.size();

	if (node->m_Parent == nullptr)
	{
		stream.write((char*)&node->m_Char, sizeof(wchar_t));
		stream.write((char*)&ChildCount, sizeof(size_t));
	}

	for (auto& TreeNode : node->m_Children)
	{
		stream.write((char*)&node->m_Char, sizeof(wchar_t));
		stream.write((char*)&ChildCount, sizeof(size_t));
	}

	for (auto& TreeNode : node->m_Children)
		WriteNodeToBinary(TreeNode, stream);
}

void CDataBase::LoadDataBase(const char* dbFilePath)
{
/*
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

	for (auto& WordTree : m_WordTrees)
	{
		m_WordTree[WordTree.first] = std::make_shared<CWordFlatTree>();
		m_WordTree[WordTree.first]->FlattenWordTree(*WordTree.second);
		m_WordTree[WordTree.first]->SetParentClasses(m_WordTree[WordTree.first]);
	}
*/

	std::ifstream File(dbFilePath, std::ios::in | std::ios::binary);

	while (File.good())
    {
        wchar_t  Char;

        if (!File.seekg(sizeof(size_t), std::ios::cur))
            break;

        File.read((char*)&Char, sizeof(wchar_t ));
        File.seekg(-sizeof(size_t) - sizeof(wchar_t), std::ios::cur);

        m_WordTree[Char] = std::make_shared<CWordFlatTree>();
        m_WordTree[Char]->LoadTree(File);
        m_WordTree[Char]->SetParentClasses(m_WordTree[Char]);
    }
}

void CDataBase::SaveDataBase(const char*path)
{
	std::ofstream File(path, std::ios::out | std::ios::binary);

	for (auto& WordTree : m_WordTree)
		WordTree.second->SaveTree(File);

	File.close();
}


bool CDataBase::WordExists(std::wstring& word, CWordFlatTree::TNode* node, int charIdx)
{
    if (charIdx == word.length())
        return node->m_WordEnd;

    if (node)
    {
        CWordFlatTree::TNode* ChildNode = node->FindChild(word.at(charIdx));

        if (!ChildNode)
			return false;

        return WordExists(word, ChildNode, ++charIdx);
    }
    else
    {
        if (m_WordTree.find(word.at(0)) == m_WordTree.end())
            return false;

        return WordExists(word, m_WordTree[word.at(0)]->GetNode(0), 1);
    }
}
