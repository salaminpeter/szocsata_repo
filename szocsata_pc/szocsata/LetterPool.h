#pragma once

#include <map>
#include <vector>

class CLetterPool
{
public:
	
	CLetterPool();

	void DealLetters(std::wstring& letters);
	void Init();
	int GetRemainingLetterCount();

	bool Empty() {return m_LetterIdx.size() == 0;}

private:
	
	std::map<wchar_t, int> m_Letters;
	std::vector<wchar_t> m_LetterIdx;
};