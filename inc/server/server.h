/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef SERVER_H
#define SERVER_H

#include "common/net/net.h"

// WatIsDeze: Has been moved to shared/shared.h for the clgame dll.
// typedef enum {
//     ss_dead,            // no map loaded
//     ss_loading,         // spawning level edicts
//     ss_game,            // actively running
//     ss_pic,             // showing static picture
//     ss_broadcast,       // running MVD client
//     ss_cinematic,
// } server_state_t;

#if USE_ICMP
void SV_ErrorEvent(netadr_t *from, int ee_errno, int ee_info);
#endif
void SV_Init(void);
void SV_Shutdown(const char *finalmsg, error_type_t type);
unsigned SV_Frame(unsigned msec);
#if USE_SYSCON
void SV_SetConsoleTitle(void);
#endif
//void SV_ConsoleOutput(const char *msg);
server_state_t	SV_GetState (void);                 // WATISDEZE: Added for client game dll
void            SV_SetState (server_state_t state); // WATISDEZE: Added for client game dll

#if USE_CLIENT
char *SV_GetSaveInfo(const char *dir);
#endif

// svgame.c


// user.c
void SV_PreRunCmd(void);
void SV_RunCmdCleanup(void);

#endif // SERVER_H
