#include "stdafx.h"
#include "ModelColorData.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>


void CModelColorData::BindBuffer(int location, int offset, bool setVertexAttrib) const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_TextureCoordBufferId);
	
	if (setVertexAttrib)
		glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset * sizeof(GLfloat)));
}

void CModelColorData::SetVertexAttribs(int location, unsigned offset)
{
	glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset * sizeof(GLfloat)));
}
