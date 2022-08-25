/***
*
*	License here.
*
*	@file
*
*	Client Game Prediction Interface Implementation.
* 
***/
#pragma once

//---------------------------------------------------------------------
// Client Game Prediction IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGamePrediction : public IClientGameExportPrediction{
public:
    //! Destructor.
    virtual ~ClientGamePrediction() = default;

    /**
    *   @brief  Checks for prediction incorectness. If found, corrects it.
    **/
    void CheckPredictionError(ClientMoveCommand* moveCommand) final;
    
    /**
    *   @brief  Adds the delta angles to the view angles. Required for other
    *           (especially rotating) objects to be able to push the player around properly.
    **/
    void PredictAngles() final;

    /**
    *   @brief  Process the actual predict movement simulation.
    **/
    void PredictMovement(uint64_t acknowledgedCommandIndex, uint64_t currentCommandIndex) final;

    /**
    *   @brief  Update the client side audio state.
    **/
    void UpdateClientSoundSpecialEffects(PlayerMove* pm) final;

private:
	/**
	*	@brief	Called by DispatchPredictTouchCallbacks to apply the current player move results
	*			to the actual player entity itself.
	**/
	void PlayerMoveToClientEntity( PlayerMove *pm );

	/**
	*	@brief	Dispatch touch callbacks for all predicted touched entities.
	**/
	void DispatchPredictedTouchCallbacks(PlayerMove *pm);
	
	/**
    *   @brief  Player Move Simulation Trace Wrapper.
    **/
    static TraceResult PM_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end);

public:
    /**
    *   @brief  Player Move Simulation PointContents Wrapper.
    **/
    static int32_t PM_PointContents(const vec3_t &point);
};

