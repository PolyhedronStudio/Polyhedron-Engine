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
        // Success by default.
        default:
            return qtrue;
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