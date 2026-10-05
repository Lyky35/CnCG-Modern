#ifndef __GSERVER_H__
#define __GSERVER_H__

/*
** gstats/gserver.h stub: declarations for the GameSpy server-browser helpers.
** Multiplayer/GameSpy services are unavailable in this build; see gserver_stub.cpp.
*/

#include "GameSpy/GP/GP.h"
#include "GameSpy/Peer/Peer.h"

#ifdef __cplusplus
extern "C" {
#endif

SBServer SBServers(PEER peer);
SBServer SBServersNext(SBServer current, SBServer *nextOut);
const char *SBServerName(SBServer server);
const char *SBServerGetStringValue(SBServer server, const char *key, const char *defaultValue);
int SBServerGetIntValue(SBServer server, const char *key, int defaultValue);
const char *SBServerGetPlayerStringValue(SBServer server, int index, const char *key, const char *defaultValue);
int SBServerGetPlayerIntValue(SBServer server, int index, const char *key, int defaultValue);
int SBServerHasBasicKeys(SBServer server);
int SBServerHasFullKeys(SBServer server);
unsigned int SBServerGetPublicInetAddress(SBServer server);
unsigned int SBServerGetPrivateInetAddress(SBServer server);
unsigned short SBServerGetPrivateQueryPort(SBServer server);
int SBServerEnumKeys(SBServer server, void (*enumFunc)(char *key, char *val, void *param), void *param);

#define ServerGetPlayerStringValue SBServerGetPlayerStringValue

#ifdef __cplusplus
}
#endif

#endif
