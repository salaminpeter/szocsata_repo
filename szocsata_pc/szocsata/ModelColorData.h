#pragma once

#include "glm\gtc\matrix_transform.hpp" //TODO ezeknel normalisan includolni, van egy csomo!!!!

#include <vector>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

class CModelColorData
{
public:
	
	virtual ~CModelColorData();
	virtual void GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions) {}

	void BindBuffer(int location, int offset, bool setVertexAttrib = true) const;
	void SetVertexAttribs(int location, unsigned offset);

protected:

	GLuint m_TextureCoordBufferId = 0;
};