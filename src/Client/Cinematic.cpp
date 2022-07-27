/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "Client.h"
#include "Client/Sound/Sound.h"
#include "Client/Sound/Vorbis.h"
#include "../Common/Files.h"
#include "Refresh/Images.h"

typedef struct
{
    byte	*data;
    int		count;
} cblock_t;

typedef struct
{
    int     s_khz_original;
    int     s_rate;
    int     s_width;
    int     s_channels;

    int     width;
    int     height;

    // order 1 huffman stuff
    int     *hnodes1;	// [256][256][2];
    int     numhnodes1[256];

    int     h_used[512];
    int     h_count[512];

    byte    palette[768];
    qboolean palette_active;

    char    file_name[MAX_QPATH];
    qhandle_t file;

    int     start_time; // cls.realtime for first cinematic frame
    int     frame_index;
} cinematics_t;

static cinematics_t cin = { };

/*
==================
SCR_StopCinematic
==================
*/
void SCR_StopCinematic(void)
{
    cin.start_time = 0;	// done

    S_UnqueueRawSamples();

    if (cl.precaches.images[0])
    {
        R_UnregisterImage(cl.precaches.images[0]);
        cl.precaches.images[0] = 0;
    }

    if (cin.file)
    {
        FS_FCloseFile(cin.file);
        cin.file = 0;
    }
    if (cin.hnodes1)
    {
        Z_Free(cin.hnodes1);
        cin.hnodes1 = NULL;
    }

    // switch the sample rate back to its original value if necessary
    if (cin.s_khz_original != 0)
    {
        Cvar_Set("s_khz", va("%d", cin.s_khz_original));
        cin.s_khz_original = 0;
    }
}

/*
====================
SCR_FinishCinematic

Called when either the cinematic completes, or it is aborted
====================
*/
// CPP: Declared in Sound.h, but now here too...
void AL_UnqueueRawSamples();
void SCR_FinishCinematic(void)
{
    SCR_StopCinematic();
	
	if(s_started == SS_OAL)
		AL_UnqueueRawSamples();

    // tell the server to advance to the next map / cinematic
    CL_ClientCommand(va("nextserver %i\n", cl.serverCount));
}

//==========================================================================

/*
==================
SmallestNode1
==================
*/
int	SmallestNode1(int numhnodes)
{
    int		i;
    int		best, bestnode;

    best = 99999999;
    bestnode = -1;
    for (i = 0; i < numhnodes; i++)
    {
        if (cin.h_used[i])
            continue;
        if (!cin.h_count[i])
            continue;
        if (cin.h_count[i] < best)
        {
            best = cin.h_count[i];
            bestnode = i;
        }
    }

    if (bestnode == -1)
        return -1;

    cin.h_used[bestnode] = true;
    return bestnode;
}


/*
==================
Huff1TableInit

Reads the 64k counts table and initializes the node trees
==================
*/
void Huff1TableInit(void)
{
    int		prev;
    int		j;
    int		*node, *nodebase;
    byte	counts[256];
    int		numhnodes;

    // CPP: Cast to int*
    cin.hnodes1 = (int*)Z_Malloc(256 * 256 * 2 * 4);
    memset(cin.hnodes1, 0, 256 * 256 * 2 * 4);

    for (prev = 0; prev < 256; prev++)
    {
        memset(cin.h_count, 0, sizeof(cin.h_count));
        memset(cin.h_used, 0, sizeof(cin.h_used));

        // read a row of counts
        FS_Read(counts, sizeof(counts), cin.file);
        for (j = 0; j < 256; j++)
            cin.h_count[j] = counts[j];

        // build the nodes
        numhnodes = 256;
        nodebase = cin.hnodes1 + prev * 256 * 2;

        while (numhnodes != 511)
        {
            node = nodebase + (numhnodes - 256) * 2;

            // pick two lowest counts
            node[0] = SmallestNode1(numhnodes);
            if (node[0] == -1)
                break;	// no more

            node[1] = SmallestNode1(numhnodes);
            if (node[1] == -1)
                break;

            cin.h_count[numhnodes] = cin.h_count[node[0]] + cin.h_count[node[1]];
            numhnodes++;
        }

        cin.numhnodes1[prev] = numhnodes - 1;
    }
}

/*
==================
Huff1Decompress
==================
*/
cblock_t Huff1Decompress(cblock_t in)
{
    byte		*input;
    byte		*out_p;
    int			nodenum;
    int			count;
    cblock_t	out;
    int			inbyte;
    int			*hnodes, *hnodesbase;
    //int		i;

        // get decompressed count
    count = in.data[0] + (in.data[1] << 8) + (in.data[2] << 16) + (in.data[3] << 24);
    input = in.data + 4;
    out_p = out.data = (byte*)Z_Malloc(count); // CPP: Cast to byte*

    // read bits

    hnodesbase = cin.hnodes1 - 256 * 2;	// nodes 0-255 aren't stored

    hnodes = hnodesbase;
    nodenum = cin.numhnodes1[0];
    while (count)
    {
        inbyte = *input++;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
        //-----------
        if (nodenum < 256)
        {
            hnodes = hnodesbase + (nodenum << 9);
            *out_p++ = nodenum;
            if (!--count)
                break;
            nodenum = cin.numhnodes1[nodenum];
        }
        nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
        inbyte >>= 1;
    }

    if (input - in.data != in.count && input - in.data != in.count + 1)
    {
        Com_Printf("Decompression overread by %li", (input - in.data) - in.count);
    }
    out.count = out_p - out.data;

    return out;
}

/*
==================
SCR_PlayCinematic

==================
*/
void SCR_PlayCinematic(const char *name)
{
    int		width, height;
    int		old_khz;

    // make sure CD isn't playing music
    OGG_Stop();

    cin.s_khz_original = 0;

    cin.frame_index = 0;
    cin.start_time = 0;

    if (!COM_CompareExtension(name, ".pcx"))
    {
        cl.precaches.images[0] = R_RegisterPic2(name);
        if (!cl.precaches.images[0]) {
            SCR_FinishCinematic();
            return;
        }
    }
    else
    {
        SCR_FinishCinematic();
        return;
    }

    cls.connectionState = ClientConnectionState::Cinematic;

    SCR_EndLoadingPlaque();     // get rid of loading plaque
    Con_Close(false);          // get rid of connection screen
}
