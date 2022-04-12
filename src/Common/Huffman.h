/***
*
*	License here.
*
*	@file
*	
*	Huffman Coding:
*	
*	Handles lossless compression and decompression of data using Huffman coding.
*	Used for network packets and demo files.
*
***/
#pragma once.

/**
*	@brief	Initializes the Huffman compressor / decompressor.
**/
void				Huff_Init (void);

/**
*	@brief	Compresses the given data.
**/
int32_t Huff_Compress (const byte *in, byte *out, int32_t  size);

/**
*	@brief	Decompresses the given data.
**/
int32_t Huff_Decompress (const byte *in, byte *out, int32_t size);