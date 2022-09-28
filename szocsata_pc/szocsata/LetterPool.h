#pragma once

#include <map>
#include <vector>

class CLetterPool
{
public:
	
	CLetterPool();

	int DealLetters(std::wstring& letters);
	void Init(bool initLettersCount = true);
	int GetRemainingLetterCount();

	bool Empty() {return m_LetterIdx.size() == 0;}
	int GetLetterCount(int idx);
	bool SetLetterCount(int idx, int count);

	size_t GetCharacterCount() {return m_LetterIdx.size();}

private:
	
	std::map<int, int> m_LettersForSize;
	std::map<wchar_t, int> m_Letters;
	std::vector<wchar_t> m_LetterIdx;
};