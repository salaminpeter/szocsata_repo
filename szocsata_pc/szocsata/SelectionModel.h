#pragma once

#include "Model.h"

class CSelectionModel : public CModel
{
public:

	CSelectionModel(std::shared_ptr<CModelPositionData> positionData) : CModel(false, 1, positionData, nullptr, "", "transparent_color") {}
};