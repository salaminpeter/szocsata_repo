#pragma once

#include "glm\gtc\matrix_transform.hpp"

#include <vector>

class CModelColorData
{
public:

	virtual void GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions) {}

	void BindBuffer(int location, int offset, bool setVertexAttrib = true) const;
	void SetVertexAttribs(int location, unsigned offset);

protected:

	unsigned long m_TextureCoordBufferId = -1;
};