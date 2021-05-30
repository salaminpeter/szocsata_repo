#pragma once

#include "Model.h"

class CSelectionModel : public CModel
{
public:

	CSelectionModel(CModelPositionData* positionData) : CModel(false, 1, positionData, nullptr, "", "transparent_color") {}
};