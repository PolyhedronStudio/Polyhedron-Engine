/***
*
*	License here.
*
*	@file
*
*	Client Game Movement Interface Implementation.
* 
***/
#pragma once

// Predeclare.
class KeyBinding;

//---------------------------------------------------------------------
// Client Game Movement IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameMovement : public IClientGameExportMovement {
public:
    friend class ClientGameCore;

    //! Destructor.
    virtual ~ClientGameMovement() = default;

    /**
    *   @brief  Called when the movement command needs to be build for the given
    *           client networking frame.
    **/
    void BuildFrameMovementCommand(int64_t miliseconds) final;

    /**
    *   @brief  Finalize the movement user command before sending it to server.
    **/
    void FinalizeFrameMovementCommand() final;

	/**
    *   @brief  Each frame we sample the user input and configure the ClientMoveCommand
    *           which is then sent to the server.
    **/
    ClientMoveCommand moveCommand = {};

private:
    /**
    *   @brief  Register input messages and binds them to a callback function.
    *           Bindings are set in the config files, and/or the options menu.
    * 
    *           For more information, it still works like in q2pro.
    **/
    void RegisterInput(void);

    /**
    *   @brief  Handles the mouse move based input adjustment.
    **/
    vec3_t BaseMove(const vec3_t& inMove);

    /**
    *   @brief  Moves the local angle positions
    **/
    void AdjustAngles(uint64_t miliseconds);

    /**
    *   @brief  Build and return the intended movement vector
    **/
    void MouseMove();

    /**
    *   @brief  Returns the clamped movement speeds.
    **/
    vec3_t ClampSpeed(const vec3_t& inMove);

    /**
    *   @brief  Ensures the Pitch is clamped to prevent camera issues.
    **/
    void ClampPitch(void);



private:


    /**
    *   Player Input Key Binds.
    **/
    static KeyBinding in_klook;
    static KeyBinding in_left, in_right, in_forward, in_back;
    static KeyBinding in_lookup, in_lookdown, in_moveleft, in_moveright;
    static KeyBinding in_strafe, in_speed, in_use, in_reload, in_primary_fire, in_secondary_fire;
    static KeyBinding in_up, in_down;

    /**
    *   Mouse Input Sampling Configuration CVars.
    **/
    static cvar_t* m_filter;
    static cvar_t* m_accel;
    static cvar_t* m_autosens;

    static cvar_t* m_pitch;
    static cvar_t* m_invert;
    static cvar_t* m_yaw;
    static cvar_t* m_forward;
    static cvar_t* m_side;

    /**
    *   Movement Speed Sampling Configuration CVars.
    **/
    static cvar_t* cl_upspeed;
    static cvar_t* cl_forwardspeed;
    static cvar_t* cl_sidespeed;
    static cvar_t* cl_yawspeed;
    static cvar_t* cl_pitchspeed;
    static cvar_t* cl_anglespeedkey;
    static cvar_t* cl_instantpacket;

    static cvar_t* cl_run; // WID: TODO: This is the new one, so it can be externed.

    static cvar_t* sensitivity;

    /**
    *   This looks silly but.
    *   TODO:   This is for mouse delta x/y sampling and is related to the user input
    *           system APIs. Look into that for cleaning this out.
    */
    static struct in_state_t {
        int         oldDeltaX;
        int         oldDeltaY;
    } inputState;

    //! Maintains state of whether to use mouse looking or not.
    static qboolean in_mlooking;

    //! The actual number supplied as an argument to the impulse cmd.
    static int32_t         in_impulse;



private:
    /**
    *   Input Sampler Command Callbacks.
    * 
    *   TODO: Rework how engine callbacks operate and allow for custom user data.
    **/
    static void IN_KLookDown(void);
    static void IN_KLookUp(void);

    static void IN_UpDown(void);
    static void IN_UpUp(void);

    static void IN_DownDown(void);
    static void IN_DownUp(void);

    static void IN_LeftDown(void);
    static void IN_LeftUp(void);

    static void IN_RightDown(void);
    static void IN_RightUp(void);

    static void IN_ForwardDown(void);
    static void IN_ForwardUp(void);

    static void IN_BackDown(void);
    static void IN_BackUp(void);

    static void IN_LookupDown(void);
    static void IN_LookupUp(void);

    static void IN_LookdownDown(void);
    static void IN_LookdownUp(void);

    static void IN_MoveleftDown(void);
    static void IN_MoveleftUp(void);

    static void IN_MoverightDown(void);
    static void IN_MoverightUp(void);

    static void IN_SpeedDown(void);
    static void IN_SpeedUp(void);

    static void IN_StrafeDown(void);
    static void IN_StrafeUp(void);

    /**
    *   +primaryfire
    **/
    static void IN_PrimaryFireDown(void);
    static void IN_PrimaryFireUp(void);

    /**
    *   +secondaryfire
    **/
    static void IN_SecondaryFireDown(void);
    static void IN_SecondaryFireUp(void);

    /**
    *   +reload
    **/
    static void IN_ReloadDown(void);
    static void IN_ReloadUp(void);

    /**
    *   +use
    **/
    static void IN_UseDown(void);
    static void IN_UseUp(void);

    /**
    *   impulse
    **/
    static void IN_Impulse(void);
    static void IN_CenterView(void);

    static void IN_MLookDown(void);
    static void IN_MLookUp(void);
};
