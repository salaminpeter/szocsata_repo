#pragma once

class CBinaryBoolList
{
public:

	bool GetFlag(int idx)
	{
		return (1 << idx) & m_BoolList;
	}

	int SetFlag(int idx, bool on)
	{
		if (on)
			m_BoolList |= (1 << idx);
		else
			m_BoolList &= (((1 << 11) - 1) ^ (1 << idx));

		return m_BoolList;
	}

	void Reset()
	{
		m_BoolList = 0;
	}

	unsigned int GetList()
	{
		return  m_BoolList;
	}

	void SetList(unsigned int boolList)
	{
		m_BoolList = boolList;
	}

private:
	
	unsigned int m_BoolList = 0;
};