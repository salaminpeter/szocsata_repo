#pragma once

#include <list>
#include <string>
#include <vector>
#include <memory>

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

class CWordFlatTree
{
public:
	
	friend class TNode;

	struct TNode
	{
		wchar_t m_Char;
		size_t m_ParentIdx;
		size_t m_ChildrenIdx;
		size_t m_ChildCount;
		size_t m_Level;
		bool m_WordEnd;
		
		std::weak_ptr<CWordFlatTree> m_ParentClass;

		TNode(wchar_t chr, size_t parentIdx, size_t childrenIdx, size_t childCount, bool wordEnd, int level) : m_Char(chr), m_ParentIdx(parentIdx), m_ChildrenIdx(childrenIdx), m_ChildCount(childCount), m_WordEnd(wordEnd), m_Level(level) {}
		TNode* FindChild(wchar_t chr);
		TNode* Parent();
	};

	void FlattenWordTree(CWordTree& wordTree);
    void SetParentClasses(std::weak_ptr<CWordFlatTree> parent);

	TNode* GetNode(size_t idx)
	{
	    if (idx < m_FlatTree.size())
            return &m_FlatTree[idx];

        return nullptr;
	}

private:

    void SaveTree(const char* path);
	void FlattenNodeChildren(CWordTree::TNode& node, size_t level, size_t parentIdx);

private:

	std::vector<TNode> m_FlatTree;
};
