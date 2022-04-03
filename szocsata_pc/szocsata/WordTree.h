#pragma once

#include <list>
#include <string>

class CWordTree
{
public:

	struct TNode
	{
		wchar_t m_Char;
		TNode* m_Parent;
		std::list<TNode*> m_Children;
		bool m_WordEnd;
		int m_Level;

		TNode(wchar_t chr, bool wordEnd, TNode* parent, int level) : m_Char(chr), m_WordEnd(wordEnd), m_Parent(parent), m_Level(level) {}
		TNode* FindChild(wchar_t chr);
	};

public:

	CWordTree(wchar_t firstChar)
	{
		m_Root = new TNode(firstChar, false, nullptr, 0);
	}

	~CWordTree()
	{
		Delete();
	}
	
	TNode* Root()
	{
		return m_Root;
	}

	void AddWord(const std::wstring& word, TNode* node, int charIdx = 1);
	void Delete(TNode* node = nullptr);

private:

	TNode* m_Root;
};
