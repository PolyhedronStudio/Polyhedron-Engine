/*
// LICENSE HERE.

//
// gamemodule.h
//
// Contains declarations of all the functions concerning interaction with
// the client game module.
//
*/
#ifndef __CLIENT_CLGMODULE_H__
#define __CLIENT_CLGMODULE_H__

// Has to be defined for these to be declared.
#ifdef USE_CLIENT

#else
//
// cgmodule.c
//

//
// Init
//
void        CL_GM_Init (void);
void        CL_GM_Shutdown (void);

//
// Parse
//
void		CL_GM_StartServerMessage (void);
qboolean	CL_GM_ParseServerMessage (int serverCommand);
qboolean    CL_GM_ParseDemoMessage (int serverCommand);
void		CL_GM_EndServerMessage (void);

//
// View
//
void		CL_GM_PreRenderView (void);
void		CL_GM_RenderView (void);
void		CL_GM_PostRenderView (void);

#endif // USE_CLIENT
#endif //  __CLIENT_CGMODULE_H__