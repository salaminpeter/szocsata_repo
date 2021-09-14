#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "Texture.h"
#include "BoardModel.h"
#include "Config.h"
#include "BoardModelData.h"

#include <vector>

CBoardBaseModel::CBoardBaseModel() : CModel(true, 3, std::make_shared<CBoardPositionData>(5), std::make_shared<CBoardColorData>(5), "boardtex.bmp", "per_pixel_light_textured") //TODO lod configbol jojjon!
{ 
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);

	m_BoundingSphereRadius = std::sqrtf(BoardSize * BoardSize * 2);
	m_PositionData->GeneratePositionBuffer();
	m_ColorData->GenerateTextureCoordBuffer(std::static_pointer_cast<CBoardPositionData>(m_PositionData)->GetVertices());
}


