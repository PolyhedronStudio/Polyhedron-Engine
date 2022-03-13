// License here.
// 
//
// ClientGamePrediction implementation.
#pragma once

#include "Shared/Interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Prediction IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGamePrediction : public IClientGameExportPrediction{
public:
    //! Destructor.
    virtual ~ClientGamePrediction() = default;

    void CheckPredictionError(ClientMoveCommand* moveCommand) final;
    void PredictAngles() final;
    void PredictMovement(uint32_t acknowledgedCommandIndex, uint32_t currentCommandIndex) final;

    void UpdateClientSoundSpecialEffects(PlayerMove* pm) final;
};

