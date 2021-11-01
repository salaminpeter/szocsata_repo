#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "LetterModel.h"
#include "Texture.h"
#include "Config.h"
#include "RoundedBoxModelData.h"

#include <vector>

CLetterModel::CLetterModel(unsigned textureOffset, int bx, int by, wchar_t c, std::shared_ptr<CRoundedBoxPositionData> positionData, std::shared_ptr<CModelColorData> colorData) :
	CModel(true, 3, positionData, colorData, "letters.bmp", "per_pixel_light_textured"),
	m_BoardX(bx),
	m_BoardY(by),
	m_Char(c),
	m_TextureOffset(textureOffset)
{
}

void CLetterModel::Position(float x, float y)
{
	ResetMatrix();
	Translate(glm::vec3(x, y, 0.f));
}

