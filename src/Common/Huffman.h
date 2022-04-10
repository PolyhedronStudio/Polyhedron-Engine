/***
*
*	License here.
*
*	@file
*	
*	Handles lossless compression and decompression of data using Huffman coding.
*	Used for network packets. (And in the future, the demo files.)
*
***/

#pragma once

// Initializes the Huffman compressor/decompressor
void				Huff_Init (void);

// Compresses the given data
int					Huff_Compress (const byte *in, byte *out, int size);

// Decompresses the given data
int					Huff_Decompress (const byte *in, byte *out, int size);
