#pragma once

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>


class CModelPositionData
{
public:
	
	virtual ~CModelPositionData();

	virtual void GeneratePositionBuffer() = 0;

	void BindVertexBuffer(int posLocation, int normalLocation) const;
	void BindIndexBuffer() const;

	unsigned GetIndexCount() const {return m_IndexCount;}
	unsigned GetVertexBufferId() {return m_VertexBufferId;}
	unsigned GetIndexBufferId() {return m_IndexBufferId; }

protected:

	GLuint m_VertexBufferId = 0;
	GLuint m_IndexBufferId;

	unsigned m_IndexCount = 0;
};