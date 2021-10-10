#pragma once

#include "Model.h"
#include "SquareModelData.h"

class CSelectionModel : public CModel
{
public:

	CSelectionModel(std::shared_ptr<CRoundedSquarePositionData> positionData) : CModel(false, 1, std::static_pointer_cast<CModelPositionData>(positionData), nullptr, "", "transparent_color") {}
};