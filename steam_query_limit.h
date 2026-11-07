#ifndef STEAM_QUERY_LIMIT_H
#define STEAM_QUERY_LIMIT_H

#include "quake_common.h"

// The limit hooks SV_HandleSteamPacket, which is only searched for in the 64-bit server.
#if defined(__x86_64__) || defined(_M_X64)
#define STEAM_QUERY_LIMIT
#endif

#ifdef STEAM_QUERY_LIMIT

// Registers the cvars of the limit with their defaults. Cvars that are already set, e.g. by
// the server.cfg or the command line, keep their value.
void SteamQueryLimit_RegisterCvars(void);

// Returns 1 if the packet may be handed to Steam, 0 if it is dropped because its source
// exceeded the limit. Only server queries (A2S_INFO, A2S_PLAYER and A2S_RULES) from IPv4
// sources are ever dropped.
int SteamQueryLimit_Allow(const netadr_t* from, const msg_t* msg);

#endif /* STEAM_QUERY_LIMIT */

#endif /* STEAM_QUERY_LIMIT_H */
