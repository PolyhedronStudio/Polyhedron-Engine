// License here.
// 
//
// ClientGamePrediction implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Prediction IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGamePrediction : public IClientGameExportPrediction{
public:
    void CheckPredictionError(ClientMoveCommand* clientMoveCommand) final;
    void PredictAngles() final;
    void PredictMovement(uint32_t acknowledgedCommandIndex, uint32_t currentCommandIndex) final;
};

