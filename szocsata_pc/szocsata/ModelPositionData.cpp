#include "stdafx.h"
#include "ModelPositionData.h"

CModelPositionData::~CModelPositionData()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_VertexBufferId);
	glDeleteBuffers(1, &m_IndexBufferId);
}


void CModelPositionData::BindVertexBuffer(int posLocation, int normalLocation) const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);

	float mul = normalLocation > 0 ? 6.f : 3.f;

	//position
	glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, mul * sizeof(GLfloat), (void*)0);
	//vertex normal
	if (normalLocation > 0)
		glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void CModelPositionData::BindIndexBuffer() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferId);
}

