// LICENSE HERE.

//
// clg_parse.c
//
//
// Handles parsing of the Server Messages.
//
#include "clg_local.h"


//
//=============================================================================
//
//	SERVER MESSAGES.
//
//=============================================================================
//

//
//===============
// CLG_ParseLayout
// 
// Parses the layout string that's passed over from the server game module
// and copies it into the client state structure.
//===============
//
static void CLG_ParseLayout(void)
{
    clgi.MSG_ReadString(cl->layout, sizeof(cl->layout));
}

//
//===============
// CLG_ParseInventory
// 
// Parses the the inventory that's sent by the server game module.
//===============
//
static void CLG_ParseInventory(void)
{
    int        i;

    for (i = 0; i < MAX_ITEMS; i++) {
        cl->inventory[i] = clgi.MSG_ReadShort();
    }
}

//
//=============================================================================
//
//	CLIENT MODULE 'PARSE' ENTRY POINTS.
//
//=============================================================================
//

//
//===============
// CLG_StartServerMessage
// 
// Called when a server message comes in.
//===============
//
void CLG_StartServerMessage (void) {

}

//
//===============
// CLG_StartServerMessage
// 
// Called when a server message hasn't been parsed by the client yet.
// Returns true on success, returns false in case of error.
//
// When returning false, the client will notify about the error.
//===============
//
qboolean CLG_ParseServerMessage (int serverCommand) {
    // Switch cmd.
    switch (serverCommand) {
        case svc_inventory:
            CLG_ParseInventory();
            return qtrue;
        break;
        case svc_layout :
            CLG_ParseLayout();
            return qtrue;
        break;
    // Fail by default.
    default:
        return qfalse;
    }
}

//
//===============
// CLG_SeekDemoMessage
//
// A variant of ParseServerMessage that skips over non-important action messages,
// used for seeking in demos.
//
// The main things that are important here, are events which influence 
// possible entities being displayed, HUD display, or having sounds played
// just to name a few examples.
//===============
//
qboolean CLG_SeekDemoMessage(int demoCommand) {
    // Switch cmd.
    switch (demoCommand) {
    case svc_inventory:
        CLG_ParseInventory();
        return qtrue;
        break;
    case svc_layout:
        CLG_ParseLayout();
        return qtrue;
        break;
    // Return false for failure in case we've reached this checkpoint.
    default:
        return qfalse;
    }
}

//
//===============
// CLG_EndServerMessage
// 
// Called when the end of parsing a server message has been reached.
//===============
//
void CLG_EndServerMessage (int realTime) {

}