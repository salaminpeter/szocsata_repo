#pragma once

#include <memory>

#include "ModelPositionData.h"
#include "ModelColorData.h"

class CRoundedSquarePositionData;

class CBoardTilesPositionData : public CModelPositionData
{
public:

	CBoardTilesPositionData(std::shared_ptr<CRoundedSquarePositionData> posData);
	void GeneratePositionBuffer() override;

private:
	
	std::shared_ptr<CRoundedSquarePositionData> m_RoundedSquarePositionData;
};


class CBoardTilesTextureData : public CModelColorData
{
public:

	void GenerateTextureCoordBuffer(const std::vector<glm::vec3>& vertexPositions) override;
};