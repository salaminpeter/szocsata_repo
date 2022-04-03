#include "stdafx.h"
#include "WordTree.h"


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
