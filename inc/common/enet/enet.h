#pragma once



void        ENET_Init(void);
void        ENET_Shutdown(void);
//void        ENET_Config(netflag_t flag);
//void        ENET_UpdateStats(void);
//
//qboolean    ENET_GetAddress(netsrc_t sock, netadr_t* adr);
//void        ENET_GetPackets(netsrc_t sock, void (*packet_cb)(void));
//qboolean    ENET_SendPacket(netsrc_t sock, const void* data,
//    size_t len, const netadr_t* to);
//
//char* ENET_AdrToString(const netadr_t* a);
//qboolean    ENET_StringToAdr(const char* s, netadr_t* a, int default_port);
//qboolean    ENET_StringPairToAdr(const char* host, const char* port, netadr_t* a);
//
//char* ENET_BaseAdrToString(const netadr_t* a);
//#define     NET_StringToBaseAdr(s, a)   ENET_StringPairToAdr(s, NULL, a)
//
//const char* ENET_ErrorString(void);
//
//void        ENET_CloseStream(netstream_t* s);
//neterr_t    ENET_Listen(qboolean listen);
//neterr_t    ENET_Accept(netstream_t* s);
//neterr_t    ENET_Connect(const netadr_t* peer, netstream_t* s);
//neterr_t    ENET_RunConnect(netstream_t* s);
//neterr_t    ENET_RunStream(netstream_t* s);
//void        ENET_UpdateStream(netstream_t* s);
//
//ioentry_t* ENET_AddFd(qsocket_t fd);
//void        ENET_RemoveFd(qsocket_t fd);
//int         ENET_Sleep(int msec);

//extern cvar_t* net_ip;
//extern cvar_t* net_port;
//
//extern netadr_t     net_from;