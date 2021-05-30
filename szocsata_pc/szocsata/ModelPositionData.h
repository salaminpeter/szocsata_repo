#pragma once


class CModelPositionData
{
public:
	
	virtual void GeneratePositionBuffer() = 0;

	void BindVertexBuffer(int posLocation, int normalLocation) const;
	void BindIndexBuffer() const;

	unsigned GetIndexCount() const {return m_IndexCount;}

protected:

	unsigned long m_VertexBufferId;
	unsigned long m_IndexBufferId;

	unsigned m_IndexCount = 0;
};