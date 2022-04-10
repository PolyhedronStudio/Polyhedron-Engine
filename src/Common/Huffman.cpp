/***
*
*	License here.
*
*	@file
*
*	Network Channels:
*	
*	A network channel handles packet compression and decompression, packet fragmentation and
*	reassembly, and out of order / duplicate suppression.
*
***/
#include "Shared/Shared.h"
#include "Common.h"


#define MAX_SYMBOL							256

#define NOT_YET_TRANSMITTED					MAX_SYMBOL
#define INTERNAL_NODE						(MAX_SYMBOL + 1)

typedef struct huffmanNode_s {
	huffmanNode_s *		parent;
	huffmanNode_s *		left;
	huffmanNode_s *		right;

	huffmanNode_s *		next;
	huffmanNode_s *		prev;

	huffmanNode_s **	head;

	int					weight;
	int					symbol;
} huffmanNode_t;

typedef struct {
	huffmanNode_t *		tree;
	huffmanNode_t *		head;
	huffmanNode_t *		tail;

	huffmanNode_t *		table[MAX_SYMBOL + 1];
	huffmanNode_t **	freeList;

	huffmanNode_t		nodeList[768];
	int					nodeListCount;

	huffmanNode_t *		nodePtrs[768];
	int					nodePtrsCount;

	int					offset;
} huffman_t;

static int				huff_counts[256] = {
	250315,  41193,   6292,   7106,   3730,   3750,   6110,  23283,  33317,   6950,   7838,   9714,   9257,	 17259,   3949,   1778,
	  8288,   1604,   1590,   1663,   1100,   1213,   1238,   1134,   1749,   1059,	  1246,   1149,   1273,   4486,   2805,   3472,
	 21819,   1159,   1670,   1066,   1043,   1012,   1053,	  1070,	  1726,    888,   1180,    850,    960,    780,   1752,   3296,
	 10630,   4514,   5881,   2685,	  4650,   3837,   2093,   1867,	  2584,   1949,   1972,    940,   1134,   1788,   1670,   1206,
	  5719,	  6128,   7222,   6654,   3710,   3795,   1492,   1524,	  2215,   1140,   1355,    971,   2180,   1248,	  1328,   1195,
	  1770,   1078,   1264,   1266,   1168,    965,   1155,   1186,	  1347,   1228,   1529,	  1600,   2617,   2048,   2546,   3275,
	  2410,   3585,   2504,   2800,   2675,   6146,   3663,   2840,	 14253,   3164,   2221,   1687,   3208,   2739,   3512,   4796,
	  4091,   3515,   5288,   4016,   7937,	  6031,   5360,   3924,	  4892,   3743,   4566,   4807,   5852,   6400,   6225,   8291,
	 23243,   7838,	  7073,   8935,   5437,   4483,   3641,   5256,	  5312,   5328,   5370,   3492,   2458,   1694,   1821,	  2121,
	  1916,   1149,   1516,   1367,   1236,   1029,   1258,   1104,	  1245,   1006,   1149,   1025,	  1241,    952,   1287,    997,
	  1713,   1009,   1187,    879,   1099,    929,   1078,    951,	  1656,	   930,   1153,   1030,   1262,   1062,   1214,   1060,
	  1621,    930,   1106,    912,   1034,    892,	  1158,    990,	  1175,    850,   1121,    903,   1087,    920,   1144,   1056,
	  3462,   2240,   4397,	 12136,   7758,   1345,   1307,   3278,	  1950,    886,   1023,   1112,   1077,   1042,   1061,   1071,
	  1484,   1001,   1096,    915,   1052,    995,   1070,    876,	  1111,    851,   1059,    805,   1112,	   923,   1103,    817,
	  1899,   1872,    976,    841,   1127,    956,   1159,    950,	  7791,    954,	  1289,    933,   1127,   3207,   1020,    927,
	  1355,    768,   1040,    745,    952,    805,   1073,	   740,	  1013,    805,   1008,    796,    996,   1057,  11457,  13504
};

static huffman_t		huff_compressor;
static huffman_t		huff_decompressor;


/*
 ==================
 Huff_AllocNode
 ==================
*/
static huffmanNode_t **Huff_AllocNode (huffman_t *huffman){

	huffmanNode_t	**node;

	if (!huffman->freeList)
		return &(huffman->nodePtrs[huffman->nodePtrsCount++]);

	node = huffman->freeList;
	huffman->freeList = (huffmanNode_t **)*node;

	return node;
}

/*
 ==================
 Huff_FreeNode
 ==================
*/
static void Huff_FreeNode (huffman_t *huffman, huffmanNode_t **node){

	*node = (huffmanNode_t *)huffman->freeList;
	huffman->freeList = node;
}

/*
 ==================
 Huff_Swap

 Swaps the location of the given nodes in the tree
 ==================
*/
static void Huff_Swap (huffman_t *huffman, huffmanNode_t *node1, huffmanNode_t *node2){

	huffmanNode_t	*parent1, *parent2;

	parent1 = node1->parent;
	parent2 = node2->parent;

	if (parent1){
		if (parent1->left == node1)
			parent1->left = node2;
		else
			parent1->right = node2;
	}
	else
		huffman->tree = node2;

	if (parent2){
		if (parent2->left == node2)
			parent2->left = node1;
		else
			parent2->right = node1;
	}
	else
		huffman->tree = node1;

	node1->parent = parent2;
	node2->parent = parent1;
}

/*
 ==================
 Huff_SwapList

 Swaps the given nodes in the linked list (updates ranks)
 ==================
*/
static void Huff_SwapList (huffman_t *huffman, huffmanNode_t *node1, huffmanNode_t *node2){

	huffmanNode_t	*node;

	node = node1->next;
	node1->next = node2->next;
	node2->next = node;

	node = node1->prev;
	node1->prev = node2->prev;
	node2->prev = node;

	if (node1->next == node1)
		node1->next = node2;
	if (node2->next == node2)
		node2->next = node1;

	if (node1->next)
		node1->next->prev = node1;
	if (node2->next)
		node2->next->prev = node2;

	if (node1->prev)
		node1->prev->next = node1;
	if (node2->prev)
		node2->prev->next = node2;
}

/*
 ==================
 Huff_Increment

 Does the increments
 ==================
*/
static void Huff_Increment (huffman_t *huffman, huffmanNode_t *node){

	huffmanNode_t	*head;

	if (!node)
		return;

	if (node->next && node->next->weight == node->weight){
		head = *node->head;
		if (head != node->parent)
			Huff_Swap(huffman, head, node);

		Huff_SwapList(huffman, head, node);
	}

	if (node->prev && node->prev->weight == node->weight)
		*node->head = node->prev;
	else {
		*node->head = NULL;

		Huff_FreeNode(huffman, node->head);
	}

	node->weight++;

	if (node->next && node->next->weight == node->weight)
		node->head = node->next->head;
	else {
		node->head = Huff_AllocNode(huffman);

		*node->head = node;
	}

	if (node->parent){
		Huff_Increment(huffman, node->parent);

		if (node->prev == node->parent){
			Huff_SwapList(huffman, node, node->parent);

			if (*node->head == node)
				*node->head = node->parent;
		}
	}
}

/*
 ==================
 Huff_AddReference
 ==================
*/
static void Huff_AddReference (huffman_t *huffman, int symbol){

	huffmanNode_t	*node1, *node2;

	// If this is the first transmission of this node
	if (huffman->table[symbol] == NULL){
		node1 = &(huffman->nodeList[huffman->nodeListCount++]);
		node2 = &(huffman->nodeList[huffman->nodeListCount++]);

		node2->symbol = INTERNAL_NODE;
		node2->weight = 1;
		node2->next = huffman->head->next;

		if (huffman->head->next){
			huffman->head->next->prev = node2;

			if (huffman->head->next->weight == 1)
				node2->head = huffman->head->next->head;
			else {
				node2->head = Huff_AllocNode(huffman);

				*node2->head = node2;
			}
		}
		else {
			node2->head = Huff_AllocNode(huffman);

			*node2->head = node2;
		}

		huffman->head->next = node2;
		node2->prev = huffman->head;

		node1->symbol = symbol;
		node1->weight = 1;
		node1->next = huffman->head->next;

		if (huffman->head->next){
			huffman->head->next->prev = node1;

			if (huffman->head->next->weight == 1)
				node1->head = huffman->head->next->head;
			else {
				node1->head = Huff_AllocNode(huffman);

				*node1->head = node2;
			}
		}
		else {
			node1->head = Huff_AllocNode(huffman);

			*node1->head = node1;
		}

		huffman->head->next = node1;
		node1->prev = huffman->head;

		node1->left = node1->right = NULL;

		if (huffman->head->parent){
			if (huffman->head->parent->left == huffman->head)
				huffman->head->parent->left = node2;
			else
				huffman->head->parent->right = node2;
		}
		else
			huffman->tree = node2;

		node2->left = huffman->head;
		node2->right = node1;

		node2->parent = huffman->head->parent;
		huffman->head->parent = node1->parent = node2;

		huffman->table[symbol] = node1;

		Huff_Increment(huffman, node2->parent);
	}
	else
		Huff_Increment(huffman, huffman->table[symbol]);
}

/*
 ==================
 Huff_Send

 Sets the prefix code for the given node
 ==================
*/
#define BIT(num)							(1ULL << (num))
static void Huff_Send (huffman_t *huffman, huffmanNode_t *node, huffmanNode_t *child, byte *data){

	if (node->parent)
		Huff_Send(huffman, node->parent, node, data);

	if (child){
		if (node->right == child)
			data[huffman->offset >> 3] |= BIT(huffman->offset & 7);
		else
			data[huffman->offset >> 3] &= ~BIT(huffman->offset & 7);

		huffman->offset++;
	}
}

/*
 ==================
 Huff_Receive

 Gets a symbol
 ==================
*/
static int Huff_Receive (huffman_t *huffman, huffmanNode_t *node, const byte *data){

	while (node && node->symbol == INTERNAL_NODE){
		if (data[huffman->offset >> 3] & BIT(huffman->offset & 7))
			node = node->right;
		else
			node = node->left;

		huffman->offset++;
	}

	if (!node)
		return 0;

	return node->symbol;
}


// ================================================================================================


/*
 ==================
 Huff_Init
 ==================
*/
void Huff_Init (void){

	int		i, j;

	// Initialize the tree/list with the not-yet-transmitted node
	//Mem_Fill(&huff_compressor, 0, sizeof(huffman_t));
	memset(&huff_compressor, 0, sizeof(huffman_t));

	huff_compressor.tree = huff_compressor.head = huff_compressor.table[NOT_YET_TRANSMITTED] = &(huff_compressor.nodeList[huff_compressor.nodeListCount++]);
	huff_compressor.tree->symbol = NOT_YET_TRANSMITTED;
	huff_compressor.tree->weight = 0;
	huff_compressor.tree->parent = huff_compressor.tree->left = huff_compressor.tree->right = NULL;
	huff_compressor.head->next = huff_compressor.head->prev = NULL;
	huff_compressor.table[NOT_YET_TRANSMITTED] = huff_compressor.tree;

	// Initialize the tree/list with the not-yet-transmitted node
	//Mem_Fill(&huff_decompressor, 0, sizeof(huffman_t));
	memset(&huff_decompressor, 0, sizeof(huffman_t));

	huff_decompressor.tree = huff_decompressor.head = huff_decompressor.tail = huff_decompressor.table[NOT_YET_TRANSMITTED] = &(huff_decompressor.nodeList[huff_decompressor.nodeListCount++]);
	huff_decompressor.tree->symbol = NOT_YET_TRANSMITTED;
	huff_decompressor.tree->weight = 0;
	huff_decompressor.tree->parent = huff_decompressor.tree->left = huff_decompressor.tree->right = NULL;
	huff_decompressor.head->next = huff_decompressor.head->prev = NULL;

	// Build the tree
	for (i = 0; i < 256; i++){
		for (j = 0; j < huff_counts[i]; j++){
			Huff_AddReference(&huff_compressor, i);
			Huff_AddReference(&huff_decompressor, i);
		}
	}
}

/*
 ==================
 Huff_Compress
 ==================
*/
int Huff_Compress (const byte *in, byte *out, int size){

	int		i;

	huff_compressor.offset = 0;

	for (i = 0; i < size; i++)
		Huff_Send(&huff_compressor, huff_compressor.table[in[i]], NULL, out);

	return (huff_compressor.offset + 7) >> 3;
}

/*
 ==================
 Huff_Decompress
 ==================
*/
int Huff_Decompress (const byte *in, byte *out, int size){

	int		i;

	huff_decompressor.offset = 0;

	for (i = 0; i < size; i++)
		out[i] = Huff_Receive(&huff_decompressor, huff_decompressor.tree, in);

	return (huff_decompressor.offset + 7) >> 3;
}
