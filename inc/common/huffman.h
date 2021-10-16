#pragma once

#include "msg.h"

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT HMAX					/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

struct node_t {
	node_t* left, * right, * parent; /* tree structure */
	node_t* next, * prev; /* doubly-linked list */
	node_t** head; /* highest ranked node in block */
	int32_t		weight;
	int32_t		symbol;
};

#define HMAX 256 /* Maximum symbol */

struct huff_t {
	int32_t			blocNode;
	int32_t			blocPtrs;

	node_t* tree;
	node_t* lhead;
	node_t* ltail;
	node_t* loc[HMAX + 1];
	node_t** freelist;

	node_t		nodeList[768];
	node_t* nodePtrs[768];
};

struct huffman_t {
	huff_t		compressor;
	huff_t		decompressor;
};

void	Huff_Compress(msg_t* buf, int32_t offset);
void	Huff_Decompress(msg_t* buf, int32_t offset);
void	Huff_Init(huffman_t* huff);
void	Huff_addRef(huff_t* huff, byte ch);
int32_t		Huff_Receive(node_t* node, int32_t* ch, byte* fin);
void	Huff_transmit(huff_t* huff, int32_t ch, byte* fout);
void	Huff_offsetReceive(node_t* node, int32_t* ch, byte* fin, int32_t* offset);
void	Huff_offsetTransmit(huff_t* huff, int32_t ch, byte* fout, int32_t* offset);
void	Huff_putBit(int32_t bit, byte* fout, int32_t* offset);
int32_t		Huff_getBit(byte* fout, int32_t* offset);

extern huffman_t clientHuffTables;

#define	SV_ENCODE_START		4
#define SV_DECODE_START		12
#define	CL_ENCODE_START		12
#define CL_DECODE_START		4