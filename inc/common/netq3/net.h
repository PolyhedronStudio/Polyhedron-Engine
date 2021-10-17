#pragma once

#define	PACKET_BACKUP	32	// number of old messages that must be kept on client and
// server for delta comrpession and ping estimation
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define	MAX_PACKET_USERCMDS		32		// max number of usercmd_t in a packet

#define	PORT_ANY			-1

#define	MAX_RELIABLE_COMMANDS	64			// max string commands buffered for restransmit

void		NET_Init(void);
void		NET_Shutdown(void);
void		NET_Restart(void);
void		NET_Config(qboolean enableNetworking);

void		NET_SendPacket(netsrc_t sock, int length, const void* data, netadr_t to);
void		q_gameabi NET_OutOfBandPrint(netsrc_t net_socket, netadr_t adr, const char* format, ...);
void		q_gameabi NET_OutOfBandData(netsrc_t sock, netadr_t adr, byte* format, int len);

qboolean	NET_CompareAdr(netadr_t a, netadr_t b);
qboolean	NET_CompareBaseAdr(netadr_t a, netadr_t b);
qboolean	NET_IsLocalAddress(netadr_t adr);
const char* NET_AdrToString(const netadr_t* a);
qboolean	NET_StringToAdr(const char* s, netadr_t* a);
qboolean	NET_GetLoopPacket(netsrc_t sock, netadr_t* net_from, msg_t* net_message);
//void		NET_Sleep(int msec);


#define	MAX_MSGLEN				16384		// max length of a message, which may
// be fragmented into multiple packets

#define MAX_DOWNLOAD_WINDOW			8		// max of eight download frames
#define MAX_DOWNLOAD_BLKSIZE		2048	// 2048 byte block chunks

