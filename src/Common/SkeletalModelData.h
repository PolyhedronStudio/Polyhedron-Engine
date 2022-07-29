/***
*
*	License here.
*
*	@file
*
*	Common Skeletal Model Functionality.
*
***/
#pragma once

/**
*	@return	Game compatible skeletal model data including: animation name, 
*			and frame data, bounding box data(per frame), and joints(name, index, parentindex)
**/
void SKM_GenerateModelData(model_t* model);

/**
*	@brief	Loads up a skeletal model configuration file and passes its buffer over
*			to the parsing process. The process tokenizes the data and generates game
*			code friendly POD to work with.
**/
bool SKM_LoadAndParseConfiguration( model_t *model, const std::string &filePath );