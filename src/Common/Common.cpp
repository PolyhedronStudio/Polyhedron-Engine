/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

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

//
// common.c -- misc functions used in client and server
//

#include "../Shared/Shared.h"

#include "Bsp.h"
#include "Cmd.h"
#include "CollisionModel.h"
#include "Common.h"
#include "CVar.h"
#include "Error.h"
#include "Field.h"
#include "Fifo.h"
#include "Files.h"
#include "Huffman.h"
#include "MDFour.h"
#include "Messaging.h"
#include "Net/Net.h"
#include "Net/NetChan.h"
//#include "pmove.h"
#include "Prompt.h"
#include "Protocol.h"
#include "Tests.h"
#include "Utilities.h"
#include "Zone.h"

#include "../Client/Client.h"
#include "../Client/Keys.h"
#include "../Server/Server.h"
#include "System/System.h"

#include <setjmp.h>

#ifdef _WIN32
#include <Windows.h>
#endif

static jmp_buf  com_abortframe;    // an ErrorType::Drop occured, exit the entire frame

static void     (*com_abort_func)(void *);
static void     *com_abort_arg;

static qboolean com_errorEntered;
static char     com_errorMsg[MAXERRORMSG]; // from Com_Printf/Com_Error

static int      com_printEntered;

static qhandle_t    com_logFile;
static qboolean     com_logNewline;

static char     **com_argv;
static int      com_argc;

cvar_t  *z_perturb;

#ifdef _DEBUG
cvar_t  *developer;
#endif
cvar_t  *timescale;
cvar_t  *fixedtime;
cvar_t  *dedicated;
cvar_t  *backdoor;
cvar_t  *com_version;

cvar_t  *logfile_enable;    // 1 = create new, 2 = append to existing
cvar_t  *logfile_flush;     // 1 = flush after each print
cvar_t  *logfile_name;
cvar_t  *logfile_prefix;

#if USE_CLIENT
cvar_t  *cl_running;
cvar_t  *cl_paused;
#endif
cvar_t  *sv_running;
cvar_t  *sv_paused;
cvar_t  *com_timedemo;
cvar_t  *com_date_format;
cvar_t  *com_time_format;
#ifdef _DEBUG
cvar_t  *com_debug_break;
#endif
cvar_t  *com_fatal_error;

cvar_t  *allow_download;
cvar_t  *allow_download_players;
cvar_t  *allow_download_models;
cvar_t  *allow_download_sounds;
cvar_t  *allow_download_maps;
cvar_t  *allow_download_textures;
cvar_t  *allow_download_pics;
cvar_t  *allow_download_others;

cvar_t  *rcon_password;



uint64_t    com_framenum;
uint64_t    com_eventTime;
uint64_t    com_localTime;
qboolean    com_initialized;
time_t      com_startTime;

#if USE_CLIENT
cvar_t  *host_speeds;

// host_speeds times
uint64_t    timeBeforeServerGame;
uint64_t    timeAfterServerGame;
uint64_t    timeBeforeClientGame;
uint64_t    timeAfterClientGame;
uint64_t    timeBeforeRefresh;
uint64_t    timeAfterRefresh;
#endif

/*
============================================================================

CLIENT / SERVER interactions

============================================================================
*/

static int          rd_target;
static char         *rd_buffer;
static size_t       rd_buffersize;
static size_t       rd_length;
static rdflush_t    rd_flush;

void Com_BeginRedirect(int target, char *buffer, size_t buffersize, rdflush_t flush)
{
    if (rd_target || !target || !buffer || !buffersize || !flush) {
        return;
    }
    rd_target = target;
    rd_buffer = buffer;
    rd_buffersize = buffersize;
    rd_flush = flush;
    rd_length = 0;
}

static void Com_AbortRedirect(void)
{
    rd_target = 0;
    rd_buffer = NULL;
    rd_buffersize = 0;
    rd_flush = NULL;
    rd_length = 0;
}

void Com_EndRedirect(void)
{
    if (!rd_target) {
        return;
    }
    rd_flush(rd_target, rd_buffer, rd_length);
    rd_target = 0;
    rd_buffer = NULL;
    rd_buffersize = 0;
    rd_flush = NULL;
    rd_length = 0;
}

static void Com_Redirect(const char *msg, size_t total)
{
    size_t length;

    while (total) {
        length = total;
        if (length > rd_buffersize) {
            length = rd_buffersize;
        }
        if (rd_length > rd_buffersize - length) {
            rd_flush(rd_target, rd_buffer, rd_length);
            rd_length = 0;
        }
        memcpy(rd_buffer + rd_length, msg, length);
        rd_length += length;
        total -= length;
    }
}

static void logfile_close(void)
{
    if (!com_logFile) {
        return;
    }

    Com_Printf("Closing console log.\n");

    FS_FCloseFile(com_logFile);
    com_logFile = 0;
}

static void logfile_open(void)
{
    char buffer[MAX_OSPATH];
    unsigned mode;
    qhandle_t f;

    mode = logfile_enable->integer > 1 ? FS_MODE_APPEND : FS_MODE_WRITE;
    if (logfile_flush->integer > 0) {
        if (logfile_flush->integer > 1) {
            mode |= FS_BUF_NONE;
        } else {
            mode |= FS_BUF_LINE;
        }
    }

    f = FS_EasyOpenFile(buffer, sizeof(buffer), mode | FS_FLAG_TEXT,
                        "logs/", logfile_name->string, ".log");
    if (!f) {
        Cvar_Set("logfile", "0");
        return;
    }

    com_logFile = f;
    com_logNewline = true;
    Com_Printf("Logging console to %s\n", buffer);
}

static void logfile_enable_changed(cvar_t *self)
{
    logfile_close();
    if (self->integer) {
        logfile_open();
    }
}

static void logfile_param_changed(cvar_t *self)
{
    if (logfile_enable->integer) {
        logfile_close();
        logfile_open();
    }
}

static size_t format_local_time(char *buffer, size_t size, const char *fmt)
{
    static struct tm cached_tm;
    static time_t cached_time;
    time_t now;
    struct tm *tm;

    if (!size) {
        return 0;
    }

    buffer[0] = 0;

    now = time(NULL);
    if (now == cached_time) {
        // avoid calling localtime() too often since it is not that cheap
        tm = &cached_tm;
    } else {
        tm = localtime(&now);
        if (!tm) {
            return 0;
        }
        cached_time = now;
        cached_tm = *tm;
    }

    return strftime(buffer, size, fmt, tm);
}

static void logfile_write(int32_t printType, const char *s)
{
    char text[MAXPRINTMSG];
    char buf[MAX_QPATH];
    char *p, *maxp;
    size_t len;
    ssize_t ret;
    int c;

    if (logfile_prefix->string[0]) {
        p = strchr(logfile_prefix->string, '@');
        if (p) {
            // expand it in place, hacky
            switch (printType) {
            case PrintType::Talk:      *p = 'T'; break;
            case PrintType::Developer: *p = 'D'; break;
            case PrintType::Warning:   *p = 'W'; break;
            case PrintType::Error:     *p = 'E'; break;
            case PrintType::Notice:    *p = 'N'; break;
            default:              *p = 'A'; break;
            }
        }
        len = format_local_time(buf, sizeof(buf), logfile_prefix->string);
        if (p) {
            *p = '@';
        }
    } else {
        len = 0;
    }

    p = text;
    maxp = text + sizeof(text) - 1;
    while (*s) {
        if (com_logNewline) {
            if (len > 0 && p + len < maxp) {
                memcpy(p, buf, len);
                p += len;
            }
            com_logNewline = false;
        }

        if (p == maxp) {
            break;
        }

        c = *s++;
        if (c == '\n') {
            com_logNewline = true;
        } else {
            c = PH_CharASCII(c);
        }

        *p++ = c;
    }
    *p = 0;

    len = p - text;
    ret = FS_Write(text, len, com_logFile);
    if (ret != len) {
        // zero handle BEFORE doing anything else to avoid recursion
        qhandle_t tmp = com_logFile;
        com_logFile = 0;
        FS_FCloseFile(tmp);
        Com_EPrintf("Couldn't write console log: %s\n", Q_ErrorString(ret));
        Cvar_Set("logfile", "0");
    }
}

#ifndef _WIN32
/*
=============
Com_FlushLogs

When called from SIGHUP handler on UNIX-like systems,
will close and reopen logfile handle for rotation.
=============
*/
void Com_FlushLogs(void)
{
    if (logfile_enable) {
        logfile_enable_changed(logfile_enable);
    }
}
#endif

void Com_SetColor(color_index_t color)
{
    if (rd_target) {
        return;
    }
    Con_SetColor(color);
    Sys_SetConsoleColor(color);
}

void Com_SetLastError(const char *msg)
{
    if (msg) {
        Q_strlcpy(com_errorMsg, msg, sizeof(com_errorMsg));
    } else {
        strcpy(com_errorMsg, "No error");
    }
}

char *Com_GetLastError(void)
{
    return com_errorMsg;
}

/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.
=============
*/
void Com_LPrintf(int32_t printType, const char *fmt, ...)
{
    va_list     argptr;
    char        msg[MAXPRINTMSG];
    size_t      len;

    // may be entered recursively only once
    if (com_printEntered >= 2) {
        return;
    }

    com_printEntered++;

    va_start(argptr, fmt);
    len = Q_vscnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (printType == PrintType::Error && !com_errorEntered && len) {
        size_t errlen = len;

        if (errlen >= sizeof(com_errorMsg)) {
            errlen = sizeof(com_errorMsg) - 1;
        }

        // save error msg
        memcpy(com_errorMsg, msg, errlen);
        com_errorMsg[errlen] = 0;

        // strip trailing '\n'
        if (com_errorMsg[errlen - 1] == '\n') {
            com_errorMsg[errlen - 1] = 0;
        }
    }

    if (rd_target) {
        Com_Redirect(msg, len);
    } else {
        switch (printType) {
        case PrintType::Talk:
            Com_SetColor(COLOR_NONE);
            break;
        case PrintType::Developer:
            Com_SetColor(COLOR_ALT);
            Com_SetColor(COLOR_ORANGE);
            break;
        case PrintType::Warning:
            Com_SetColor(COLOR_ORANGE);
            break;
        case PrintType::Error:
            Com_SetColor(COLOR_RED);
            break;
        case PrintType::Notice:
            Com_SetColor(COLOR_CYAN);
            break;
        default:
            break;
        }

        // graphical console
        Con_Print(msg);

        // debugging console
        Sys_ConsoleOutput(msg);

#ifdef _WIN32
		OutputDebugStringA(msg);
#endif

        // remote console
        //SV_ConsoleOutput(msg);

        // logfile
        if (com_logFile) {
            logfile_write(printType, msg);
        }

        if (printType) {
            Com_SetColor(COLOR_NONE);
        }
    }

    com_printEntered--;
}


/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Error(int32_t errorType, const char *fmt, ...)
{
    char            msg[MAXERRORMSG];
    va_list         argptr;
    size_t          len;

    // Prevent disconnecting to a black menu etc.
    #if CLIENT
    //if(code == ErrorType::Disconnect) {
    //    // Let's go haha.
    //    CL_OpenBSPMenu(true);
    //    CL_ForwardToServer();
    //}
    #endif

    // may not be entered recursively
    if (com_errorEntered) {
#ifdef _DEBUG
        if (com_debug_break && com_debug_break->integer) {
            Sys_DebugBreak();
        }
#endif
        Sys_Error("recursive error after: %s", com_errorMsg);
    }

    com_errorEntered = true;

    va_start(argptr, fmt);
    len = Q_vscnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    // save error msg
    // can't print into it directly since it may
    // overlap with one of the arguments!
    memcpy(com_errorMsg, msg, len + 1);

    // fix up drity message buffers
    MSG_Init();

    // abort any console redirects
    Com_AbortRedirect();

    // call custom cleanup function if set
    if (com_abort_func) {
        com_abort_func(com_abort_arg);
        com_abort_func = NULL;
    }

    // reset Com_Printf recursion level
    com_printEntered = 0;

    if (errorType == ErrorType::Disconnect || errorType == ErrorType::Reconnect) {
        Com_WPrintf("%s\n", com_errorMsg);
        SV_Shutdown(va("Server was killed: %s\n", com_errorMsg), errorType);
        CL_Disconnect(errorType);
        goto abort;
    }

#ifdef _DEBUG
    if (com_debug_break && com_debug_break->integer) {
        Sys_DebugBreak();
    }
#endif

    // make otherwise non-fatal errors fatal
    if (com_fatal_error && com_fatal_error->integer) {
        errorType = ErrorType::Fatal;
    }

    if (errorType == ErrorType::Drop) {
        Com_EPrintf("********************\n"
                    "ERROR: %s\n"
                    "********************\n", com_errorMsg);
        SV_Shutdown(va("Server crashed: %s\n", com_errorMsg), ErrorType::Drop);
        CL_Disconnect(ErrorType::Drop);
        goto abort;
    }

    if (com_logFile) {
        FS_FPrintf(com_logFile, "FATAL: %s\n", com_errorMsg);
    }

    SV_Shutdown(va("Server fatal crashed: %s\n", com_errorMsg), ErrorType::Fatal);
    CL_Shutdown();
    NET_Shutdown();
    logfile_close();
    FS_Shutdown();

    Sys_Error("%s", com_errorMsg);
    // doesn't get there

abort:
    if (com_logFile) {
        FS_Flush(com_logFile);
    }
    com_errorEntered = false;
    longjmp(com_abortframe, -1);
}

void Com_AbortFunc(void (*func)(void *), void *arg)
{
    com_abort_func = func;
    com_abort_arg = arg;
}

#ifdef _WIN32
void Com_AbortFrame(void)
{
    longjmp(com_abortframe, -1);
}
#endif

/*
=============
Com_Quit

Both client and server can use this, and it will
do the apropriate things. This function never returns.
=============
*/
void Com_Quit(const char *reason, int32_t errorType)
{
    char buffer[MAX_STRING_CHARS];
    const char* what = (errorType == ErrorType::Disconnect ? "restarted" : "quit"); // C++20: char *what = type == ErrorType::Reconnect ? "restarted" : "quit";

    if (reason && *reason) {
        Q_snprintf(buffer, sizeof(buffer),
                   "Server %s: %s\n", what, reason);
    } else {
        Q_snprintf(buffer, sizeof(buffer),
                   "Server %s\n", what);
    }

    SV_Shutdown(buffer, errorType);
    CL_Shutdown();
    NET_Shutdown();
    logfile_close();
    FS_Shutdown();

    Sys_Quit();
    // doesn't get there
}

static void Com_Quit_f(void)
{
    Com_Quit(Cmd_Args(), ErrorType::Disconnect);
}

#if !USE_CLIENT
static void Com_Recycle_f(void)
{
    Com_Quit(Cmd_Args(), ErrorType::Reconnect);
}
#endif

/*
==============================================================================

                        INIT / SHUTDOWN

==============================================================================
*/

size_t Com_Time_m(char *buffer, size_t size)
{
    return format_local_time(buffer, size, com_time_format->string);
}

static size_t Com_Date_m(char *buffer, size_t size)
{
    return format_local_time(buffer, size, com_date_format->string);
}

size_t Com_Uptime_m(char *buffer, size_t size)
{
    return Com_TimeDiff(buffer, size, &com_startTime, time(NULL));
}

size_t Com_UptimeLong_m(char *buffer, size_t size)
{
    return Com_TimeDiffLong(buffer, size, &com_startTime, time(NULL));
}

static size_t Com_Random_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%d", rand_byte() % 10);
}

static size_t Com_MapList_m(char *buffer, size_t size)
{
    int i, numFiles;
    void **list;
    char *s, *p;
    size_t len, total = 0;

    list = FS_ListFiles("maps", ".bsp", 0, &numFiles);
    for (i = 0; i < numFiles; i++) {
        s = (char*)list[i]; // CPP: Cast
        p = COM_FileExtension((const char*)list[i]); // CPP: Cast
        *p = 0;
        len = strlen(s);
        if (total + len + 1 < size) {
            memcpy(buffer + total, s, len);
            buffer[total + len] = ' ';
            total += len + 1;
        }
        Z_Free(s);
    }
    buffer[total] = 0;

    Z_Free(list);
    return total;
}

static void Com_LastError_f(void)
{
    Com_Printf("%s\n", com_errorMsg);
}

#if 0
static void Com_Setenv_f(void)
{
    int argc = Cmd_Argc();

    if (argc > 2) {
        Q_setenv(Cmd_Argv(1), Cmd_ArgsFrom(2));
    } else if (argc == 2) {
        char *env = getenv(Cmd_Argv(1));

        if (env) {
            Com_Printf("%s=%s\n", Cmd_Argv(1), env);
        } else {
            Com_Printf("%s undefined\n", Cmd_Argv(1));
        }
    } else {
        Com_Printf("Usage: %s <name> [value]\n", Cmd_Argv(0));
    }
}
#endif

void Com_Address_g(genctx_t *ctx)
{
    int i;
    cvar_t *var;

    for (i = 0; i < 1024; i++) {
        var = Cvar_FindVar(va("adr%d", i));
        if (!var) {
            break;
        }
        if (!var->string[0]) {
            continue;
        }
        if (!Prompt_AddMatch(ctx, var->string)) {
            break;
        }
    }
}

void Com_Generic_c(genctx_t *ctx, int argnum)
{
    xcompleter_t c;
    xgenerator_t g;
    cvar_t *var;
    const char *s;

    // complete command, alias or cvar name
    if (!argnum) {
        Cmd_Command_g(ctx);
        Cvar_Variable_g(ctx);
        Cmd_Alias_g(ctx);
        return;
    }

    // protect against possible duplicates
    ctx->ignoredups = true;

    s = Cmd_Argv(ctx->argnum - argnum);

    // complete command argument or cvar value
    if ((c = Cmd_FindCompleter(s)) != NULL) {
        c(ctx, argnum);
    } else if (argnum == 1 && (var = Cvar_FindVar(s)) != NULL) {
        g = var->generator;
        if (g) {
            ctx->data = var;
            g(ctx);
        }
    }
}

#if USE_CLIENT
void Com_Color_g(genctx_t *ctx)
{
    int color;

    for (color = 0; color < 8; color++) {
        if (!Prompt_AddMatch(ctx, colorNames[color])) {
            break;
        }
    }
}
#endif

/*
===============
Com_AddEarlyCommands

Adds command line parameters as script statements.
Commands lead with a +, and continue until another +

Set commands are added early, so they are guaranteed to be set before
the client and server initialize for the first time.

Other commands are added late, after all initialization is complete.
===============
*/
static void Com_AddEarlyCommands(qboolean clear)
{
    int     i;
    char    *s;

    for (i = 1; i < com_argc; i++) {
        s = com_argv[i];
        if (!s) {
            continue;
        }
        if (strcmp(s, "+set")) {
            continue;
        }
        if (i + 2 >= com_argc) {
            Com_Printf("Usage: +set <variable> <value>\n");
            com_argc = i;
            break;
        }
        Cvar_SetEx(com_argv[i + 1], com_argv[i + 2], FROM_CMDLINE);
        if (clear) {
            com_argv[i] = com_argv[i + 1] = com_argv[i + 2] = NULL;
        }
        i += 2;
    }
}

/*
=================
Com_AddLateCommands

Adds command line parameters as script statements
Commands lead with a + and continue until another +

Returns true if any late commands were added, which
will keep the demoloop from immediately starting

Assumes +set commands are already filtered out
=================
*/
static qboolean Com_AddLateCommands(void)
{
    int     i;
    char    *s;
    qboolean ret = false;

    for (i = 1; i < com_argc; i++) {
        s = com_argv[i];
        if (!s) {
            continue;
        }
        if (*s == '+') {
            if (ret) {
                Cbuf_AddText(&cmd_buffer, "\n");
            }
            s++;
        } else if (ret) {
            Cbuf_AddText(&cmd_buffer, " ");
        }
        Cbuf_AddText(&cmd_buffer, s);
        ret = true;
    }

    if (ret) {
        Cbuf_AddText(&cmd_buffer, "\n");
        Cbuf_Execute(&cmd_buffer);
    }

    return ret;
}

void Com_AddConfigFile(const char *name, unsigned flags)
{
    qerror_t ret;

    ret = Cmd_ExecuteFile(name, flags);
    if (ret == Q_ERR_SUCCESS) {
        Cbuf_Execute(&cmd_buffer);
    } else if (ret != Q_ERR_NOENT) {
        Com_WPrintf("Couldn't exec %s: %s\n", name, Q_ErrorString(ret));
    }
}

/*
=================
Qcommon_Init
=================
*/
void Qcommon_Init(int argc, char **argv)
{
    if (setjmp(com_abortframe))
        Sys_Error("Error during initialization: %s", com_errorMsg);

    com_argc = argc;
    com_argv = argv;

    Com_SetLastError(NULL);

    srand(time(NULL));

    // prepare enough of the subsystems to handle
    // cvar and command buffer management
    Z_Init();
    MSG_Init();
    Cbuf_Init();
    Cmd_Init();
    Cvar_Init();
    Key_Init();
    Prompt_Init();
    Con_Init();

    //
    // init commands and vars
    //
    z_perturb = Cvar_Get("z_perturb", "0", 0);
#if USE_CLIENT
    host_speeds = Cvar_Get("host_speeds", "0", 0);
#endif
#ifdef _DEBUG
    developer = Cvar_Get("developer", "1", 0);
#else
//    developer = Cvar_Get("developer", "0", 0);
#endif
    timescale = Cvar_Get("timescale", "1", CVAR_CHEAT);
    fixedtime = Cvar_Get("fixedtime", "0", CVAR_CHEAT);
    logfile_enable = Cvar_Get("logfile", "1", 0);
    logfile_flush = Cvar_Get("logfile_flush", "1", 0);
    logfile_name = Cvar_Get("logfile_name", "console", 0);
    logfile_prefix = Cvar_Get("logfile_prefix", "[%Y-%m-%d %H:%M] ", 0);
#if USE_CLIENT
    dedicated = Cvar_Get("dedicated", "0", CVAR_NOSET);
	backdoor = Cvar_Get("backdoor", "0", CVAR_ARCHIVE);
    cl_running = Cvar_Get("cl_running", "0", CVAR_ROM);
    cl_paused = Cvar_Get("cl_paused", "0", CVAR_ROM);
#else
    dedicated = Cvar_Get("dedicated", "1", CVAR_ROM);
#endif
    sv_running = Cvar_Get("sv_running", "0", CVAR_ROM);
    sv_paused = Cvar_Get("sv_paused", "0", CVAR_ROM);
    com_timedemo = Cvar_Get("timedemo", "0", CVAR_CHEAT);
    com_date_format = Cvar_Get("com_date_format", "%Y-%m-%d", 0);
#ifdef _WIN32
    com_time_format = Cvar_Get("com_time_format", "%H.%M", 0);
#else
    com_time_format = Cvar_Get("com_time_format", "%H:%M", 0);
#endif
#ifdef _DEBUG
    com_debug_break = Cvar_Get("com_debug_break", "0", 0);
#endif
    com_fatal_error = Cvar_Get("com_fatal_error", "0", 0);
    com_version = Cvar_Get("version", com_version_string, CVAR_SERVERINFO | CVAR_USERINFO | CVAR_ROM);

    allow_download = Cvar_Get("allow_download", COM_DEDICATED ? "0" : "1", CVAR_ARCHIVE);
    allow_download_players = Cvar_Get("allow_download_players", "1", CVAR_ARCHIVE);
    allow_download_models = Cvar_Get("allow_download_models", "1", CVAR_ARCHIVE);
    allow_download_sounds = Cvar_Get("allow_download_sounds", "1", CVAR_ARCHIVE);
    allow_download_maps = Cvar_Get("allow_download_maps", "1", CVAR_ARCHIVE);
    allow_download_textures = Cvar_Get("allow_download_textures", "1", CVAR_ARCHIVE);
    allow_download_pics = Cvar_Get("allow_download_pics", "1", CVAR_ARCHIVE);
    allow_download_others = Cvar_Get("allow_download_others", "0", 0);

    rcon_password = Cvar_Get("rcon_password", "", CVAR_ARCHIVE);

    Cmd_AddCommand("z_stats", Z_Stats_f);

    //Cmd_AddCommand("setenv", Com_Setenv_f);

    Cmd_AddMacro("com_date", Com_Date_m);
    Cmd_AddMacro("com_time", Com_Time_m);
    Cmd_AddMacro("com_uptime", Com_Uptime_m);
    Cmd_AddMacro("com_uptime_long", Com_UptimeLong_m);
    Cmd_AddMacro("random", Com_Random_m);
    Cmd_AddMacro("com_maplist", Com_MapList_m);

    // add any system-wide configuration files
    Sys_AddDefaultConfig();

    // we need to add the early commands twice, because
    // a basedir or cddir needs to be set before execing
    // config files, but we want other parms to override
    // the settings of the config files
    Com_AddEarlyCommands(false);

    Sys_Init();

    Sys_RunConsole();

    FS_Init();

    Sys_RunConsole();

    // no longer allow CVAR_NOSET modifications
    com_initialized = true;

    // after FS is initialized, open logfile
    logfile_enable->changed = logfile_enable_changed;
    logfile_flush->changed = logfile_param_changed;
    logfile_name->changed = logfile_param_changed;
    logfile_enable_changed(logfile_enable);

    // execute configs: default.cfg and q2rtx.cfg may come from the packfile, but config.cfg
    // and autoexec.cfg must be real files within the game directory
	Com_AddConfigFile(COM_DEFAULT_CFG, FS_TYPE_REAL); // WID: We don't use PACK or PKZ right now. And def not for text config files.
	Com_AddConfigFile(COM_POLY_CFG, FS_TYPE_REAL); // WID: We don't use PACK or PKZ right now. And def not for text config files.
    Com_AddConfigFile(COM_CONFIG_CFG, FS_TYPE_REAL | FS_PATH_GAME);
    Com_AddConfigFile(COM_AUTOEXEC_CFG, FS_TYPE_REAL | FS_PATH_GAME);
    Com_AddConfigFile(COM_POSTEXEC_CFG, FS_TYPE_REAL);

    Com_AddEarlyCommands(true);

    Cmd_AddCommand("lasterror", Com_LastError_f);

    Cmd_AddCommand("quit", Com_Quit_f);
#if !USE_CLIENT
    Cmd_AddCommand("recycle", Com_Recycle_f);
#endif

    // Print the engine version early so that it's definitely included in the console log.
    // The log file is opened during the execution of one of the config files above.
    Com_LPrintf(PrintType::Notice, "\nEngine version: " APPLICATION " " LONG_VERSION_STRING ", built on " __DATE__ "\n\n");

    Netchan_Init();
	Huff_Init();
    NET_Init();
    BSP_Init();
    CM_Init();
    SV_Init();
    CL_Init();
    TST_Init();

    Sys_RunConsole();

    // add + commands from command line
    if (!Com_AddLateCommands()) {
        // if the user didn't give any commands, run default action
        char *cmd = (char*)(COM_DEDICATED ? "dedicated_start" : "client_start"); // C++ 20: char *cmd = COM_DEDICATED ? "dedicated_start" : "client_start";

        if ((cmd = Cmd_AliasCommand(cmd)) != NULL) { // C++ 20: .c_str()
            Cbuf_AddText(&cmd_buffer, cmd); // C++20: .c_str()
            Cbuf_Execute(&cmd_buffer);
        }
    } else {
        // the user asked for something explicit
        // so drop the loading plaque
        SCR_EndLoadingPlaque();
    }

    // even not given a starting map, dedicated server starts
    // listening for rcon commands (create socket after all configs
    // are executed to make sure port number is properly set)
    if (COM_DEDICATED || backdoor->integer) {
        NET_Config(NET_SERVER);
    }

    Com_AddConfigFile(COM_POSTINIT_CFG, FS_TYPE_REAL);

    // PH: Tell us whether we have an initialized dedicated server, or a client that has been initialized.
    if (!dedicated->integer) {
        Com_Printf("====== " PRODUCT " - Client Initialized ======\n\n");
    } else {
        Com_Printf("====== " PRODUCT " - Dedicated Server Initialized ======\n\n");
    }
    Com_LPrintf(PrintType::Notice, APPLICATION " " VERSION_STRING ", " __DATE__ "\n");

    time(&com_startTime);

    com_eventTime = Sys_Milliseconds();
}

/*
=================
Qcommon_Frame
=================
*/
void Qcommon_Frame(void)
{
#if USE_CLIENT
    uint64_t timeBefore, timeEvent, timeBetween, timeAfter;
    uint64_t clientRemainingTime;
#endif
    uint64_t oldtime, msec;
    static uint64_t serverRemainingTime;
    static double frac;

    if (setjmp(com_abortframe)) {
        return;            // an ErrorType::Drop was thrown
    }

#if USE_CLIENT
    timeBefore = timeEvent = timeBetween = timeAfter = 0;

    if (host_speeds->integer)
        timeBefore = Sys_Milliseconds();
#endif

    // sleep on network sockets when running a dedicated server
    // still do a select(), but don't sleep when running a client!
    NET_Sleep(serverRemainingTime);

    // calculate time spent running last frame and sleeping
    oldtime = com_eventTime;
    com_eventTime = Sys_Milliseconds();
    if (oldtime > com_eventTime) {
        oldtime = com_eventTime;
    }
    msec = com_eventTime - oldtime;

#if USE_CLIENT
    // spin until msec is non-zero if running a client
    if (!dedicated->integer && !com_timedemo->integer) {
        while (msec < 1) {
            qboolean break_now = CL_ProcessEvents();
            com_eventTime = Sys_Milliseconds();
            msec = com_eventTime - oldtime;
            if (break_now)
                break;
        }
    }
#endif

    if (msec > 250) {
        Com_DPrintf("Hitch warning: %u msec frame time\n", msec);
        msec = 100; // time was unreasonable,
        // host OS was hibernated or something
    }

    if (fixedtime->integer) {
        Cvar_ClampInteger(fixedtime, 1, 1000);
        msec = fixedtime->integer;
    } else if (timescale->value > 0) {
        frac += msec * timescale->value;
        msec = frac;
        frac -= msec;
    }

    // run local time
    com_localTime += msec;
    com_framenum++;

#if USE_CLIENT
    if (host_speeds->integer)
        timeEvent = Sys_Milliseconds();
#endif

    // run system console
    Sys_RunConsole();

    NET_UpdateStats();

    serverRemainingTime = SV_Frame(msec);

#if USE_CLIENT
    if (host_speeds->integer)
        timeBetween = Sys_Milliseconds();

    clientRemainingTime = CL_Frame(msec);
    if (serverRemainingTime > clientRemainingTime) {
        serverRemainingTime = clientRemainingTime;
    }

    if (host_speeds->integer)
        timeAfter = Sys_Milliseconds();

    if (host_speeds->integer) {
        //int64_t all, ev, sv, gm, cl, rf;

        int64_t all = timeAfter - timeBefore;
        int64_t ev = timeEvent - timeBefore;
        int64_t sv = timeBetween - timeEvent;
        int64_t cl = timeAfter - timeBetween;
        int64_t svgm = timeAfterServerGame - timeBeforeServerGame;
        int64_t clgm = timeAfterClientGame - timeBeforeClientGame;
        int64_t rf = timeAfterRefresh - timeBeforeRefresh;
        sv -= svgm;
        cl -= rf - clgm;

        Com_Printf("all:%3i ev:%3i sv:%3i svgm:%3i cl:%3i rf:%3i clgm:%3i\n",
                   all, ev, sv, svgm, cl, rf, clgm);
    }
#endif
}

