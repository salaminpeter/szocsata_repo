#pragma once

#include <memory>
#include <vector>
#include "glm\gtc\matrix_transform.hpp"

#include "Model.h"

class CBoardTilesPositionData;
class CBoardTilesTextureData;
class CRoundedSquarePositionData;


class CBoardTiles : public CModel
{
public:

	CBoardTiles(std::shared_ptr<CBoardTilesPositionData> positionData, std::shared_ptr<CBoardTilesTextureData> textureData, CModel* parent);

	glm::vec2 GetTilePosition(int x, int y);
};