#include "clg_local.h"

////////////////////////
// THIS FILE IS TEMPORARILY, JUST TESTING.
//////////////////
cvar_t *cl_cvar_test_a;
cvar_t *cl_cvar_test_b;

void CLG_TestCommand(void) {
    Com_DPrint("Executing test command - argc = %i\n", clgi.Cmd_Argc());
    for (int i = 0; i < clgi.Cmd_Argc(); i++) {
        Com_DPrint("Arg #%i = %s\n", i, clgi.Cmd_Argv(i));
    }
    Com_DPrint("%s", clgi.Cmd_Args());
}

void CLG_ExecuteTests(void) {
    // 
    // Test for custom commands.
    //
    clgi.Cvar_Set("maplist", "map_a map_b map_c map_d");
    clgi.Cmd_AddCommand("testcommand", CLG_TestCommand);
    clgi.Cbuf_AddText("testcommand number1 number2 number3 haha");
    clgi.Cbuf_AddText("echo \n"); // We want that newline! lol
    clgi.Cbuf_Execute();

    //
    // Cvar test.
    //
    cl_cvar_test_a = clgi.Cvar_Get("cl_cvar_test_a", "Testing cl_cvar_test_a value!", FROM_CODE);
    if (!clgi.Cvar_Exists("cl_cvar_test_b", qtrue)) {
        // Create if it doesn't exist.
        cl_cvar_test_b = clgi.Cvar_Get("cl_cvar_test_b", "Testing cl_cvar_test_b value!", FROM_CODE);
    }

    //
    // Test for forwarding cmd's to server.
    //
    clgi.Cbuf_AddText("echo Hello from trap_Cbuf_AddText and trap_CL_ForwardToServer\n");
    clgi.CL_ForwardToServer();
}