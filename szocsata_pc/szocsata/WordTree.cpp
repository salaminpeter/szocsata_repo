#include "stdafx.h"
#include "WordTree.h"

#include <algorithm>
#include <limits>
#include <fstream>

#undef max //TODO!!! numeric limits nem megy miatta

void CWordTree::Delete(TNode* node)
{
	if (!node)
		node = m_Root;

	for (auto it = node->m_Children.begin(); it != node->m_Children.end(); ++it)
	{
		Delete(*it);
		delete *it;
	}
}

void CWordTree::AddWord(const std::wstring& word, TNode* node, int charIdx)
{
	if (word.length() == 1)
		node->m_WordEnd = 1;

	if (charIdx == word.length())
		return;

	TNode* CharNode = nullptr;

	for (auto it = node->m_Children.begin(); it != node->m_Children.end(); ++it)
	{
		if ((*it)->m_Char == word[charIdx])
		{
			CharNode = *it;
			if (charIdx == word.length() - 1)
				CharNode->m_WordEnd = true;
			break;
		}
	}

	if (!CharNode)
	{
		node->m_Children.emplace_back(new TNode(word[charIdx], charIdx == word.length() - 1, node, charIdx));
		CharNode = node->m_Children.back();
	}

	AddWord(word, CharNode, ++charIdx);
}


CWordTree::TNode* CWordTree::TNode::FindChild(wchar_t chr)
{
	for (auto it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		if ((*it)->m_Char == chr)
			return (*it);
	}

	return nullptr;
}

CWordFlatTree::TNode* CWordFlatTree::TNode::Parent()
{
    if (m_ParentIdx == std::numeric_limits<size_t>::max())
        return nullptr;

	return &m_ParentClass.lock()->m_FlatTree[m_ParentIdx];
}

CWordFlatTree::TNode* CWordFlatTree::TNode::FindChild(wchar_t chr)
{
	auto ParentClassPtr = m_ParentClass.lock();

	for (size_t i = 0; i < m_ChildCount; ++i)
	{
		if (ParentClassPtr->m_FlatTree[m_ChildrenIdx + i].m_Char == chr)
			return &ParentClassPtr->m_FlatTree[m_ChildrenIdx + i];
	}

	return nullptr;
}

void CWordFlatTree::FlattenNodeChildren(CWordTree::TNode& node, size_t level, size_t parentIdx)
{
	for (auto ChildNode : node.m_Children)
		m_FlatTree.emplace_back(ChildNode->m_Char, parentIdx, 0, ChildNode->m_Children.size(), ChildNode->m_WordEnd, level + 1);

	size_t Idx = m_FlatTree.size() - node.m_Children.size();

	for (auto ChildNode : node.m_Children)
		FlattenNodeChildren(*ChildNode, level + 1, Idx++);

	//set childisx values
	if (level == 0)
	{
		size_t ParentIdx = std::numeric_limits<size_t>::max();

		for (size_t i = 0; i < m_FlatTree.size(); ++i)
		{
			if (m_FlatTree[i].m_ParentIdx != ParentIdx)
			{
				ParentIdx = m_FlatTree[i].m_ParentIdx;
				m_FlatTree[ParentIdx].m_ChildrenIdx = i;
			}
		}
	}
}

void CWordFlatTree::SetParentClasses(std::weak_ptr<CWordFlatTree> parent)
{
    for (auto& Node : m_FlatTree)
        Node.m_ParentClass = parent;
}

void CWordFlatTree::FlattenWordTree(CWordTree& wordTree)
{
	//add root
	m_FlatTree.emplace_back(wordTree.Root()->m_Char, std::numeric_limits<size_t>::max(), 1, wordTree.Root()->m_Children.size(), false, 0);

	FlattenNodeChildren(*wordTree.Root(), 0, 0);
}

void CWordFlatTree::LoadTree(std::ifstream& file)
{
	size_t TreeSize;
	file.read((char*)&TreeSize, sizeof(size_t));

	if (!file.good())
        return;

	m_FlatTree.reserve(TreeSize);

	for (int i = 0; i < TreeSize; ++i)
	{
		wchar_t  Char;
		size_t ParentIdx;
		size_t ChildrenIdx;
		size_t ChildCount;
		size_t Level;
		bool WordEnd;

		file.read((char*)&Char, sizeof(wchar_t ));
		file.read((char*)&ParentIdx, sizeof(size_t));
		file.read((char*)&ChildrenIdx, sizeof(size_t));
		file.read((char*)&ChildCount, sizeof(size_t));
		file.read((char*)&Level, sizeof(size_t));
		file.read((char*)&WordEnd, sizeof(bool));

		m_FlatTree.emplace_back(Char, ParentIdx, ChildrenIdx, ChildCount, WordEnd, Level);
	}

}

void CWordFlatTree::SaveTree(std::ofstream& file)
{
	size_t TreeSize = m_FlatTree.size();
	file.write((char*)&TreeSize, sizeof(size_t));

	for (auto& Node : m_FlatTree)
	{
		file.write((char*)&Node.m_Char, sizeof(wchar_t ));
		file.write((char*)&Node.m_ParentIdx, sizeof(size_t));
		file.write((char*)&Node.m_ChildrenIdx, sizeof(size_t));
		file.write((char*)&Node.m_ChildCount, sizeof(size_t));
		file.write((char*)&Node.m_Level, sizeof(size_t));
		file.write((char*)&Node.m_WordEnd, sizeof(bool));
	}

}
