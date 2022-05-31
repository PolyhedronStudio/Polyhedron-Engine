/***
*
*	License here.
*
*	@file
*
*	Brings Model loading to the server, allowing it to pass a model pointer
*	to the SharedGame Skeletal Animatio Data loading system.
* 
***/
#include "Server.h"

// TODO: Move elsewhere, also for MD2 and MD3.
qerror_t MOD_LoadIQM_Base(model_t* model, const void* rawdata, size_t length, const char* mod_name);

/**
*	The following is compile only with dedicated server builds.
*
*	For a client build, it has them defined elsewhere.
**/
#if USE_SERVER && !USE_CLIENT
//model_t      r_models[];
//int          r_numModels;

int registration_sequence;
#endif


/**
*	@brief	Loads in an IQM model that can be used server-side.
**/
qerror_t Model_LoadIQM(model_t* model, const void* rawdata, size_t length, const char* mod_name) {
	Hunk_Begin(&model->hunk, 0x4000000);
	model->type = model_t::MOD_ALIAS;

	qerror_t res = MOD_LoadIQM_Base(model, rawdata, length, mod_name);

	if (res != Q_ERR_SUCCESS) 	{
		Hunk_Free(&model->hunk);
		return res;
	}

	Hunk_End(&model->hunk);

	return Q_ERR_SUCCESS;
}