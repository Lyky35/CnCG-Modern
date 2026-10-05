/*
**	No-op implementations for the GameSpy Peer SDK surface declared in the stub
**	GameSpy/Peer/Peer.h. Multiplayer/GameSpy services are unavailable in this build;
**	these definitions exist so the engine links and all calls degrade gracefully.
*/

#include "GameSpy/Peer/Peer.h"
#include <string.h>

PEER peerInitialize(PEERCallbacks *callbacks) { (void)callbacks; return NULL; }
void peerShutdown(PEER peer) { (void)peer; }
PEERBool peerConnect(PEER peer, const char *nick, int profileID, peerNickErrorCallback nickCallback,
                     peerConnectCallback connectCallback, void *param, PEERBool useGP)
{ (void)peer; (void)nick; (void)profileID; (void)nickCallback; (void)connectCallback; (void)param; (void)useGP; return PEERFalse; }
void peerRetryWithNick(PEER peer, const char *nick) { (void)peer; (void)nick; }
void peerDisconnect(PEER peer) { (void)peer; }
void peerThink(PEER peer) { (void)peer; }
PEERBool peerIsConnected(PEER peer) { (void)peer; return PEERFalse; }
void peerStateChanged(PEER peer) { (void)peer; }
int peerStateChange(void) { return 0; }

PEERBool peerSetTitle(PEER peer, const char *title, const char *description, const char *gameDesc,
                  const char *secretKey, int version, PEERBool pingRooms[16], PEERBool crossPingRooms[16])
{ (void)peer; (void)title; (void)description; (void)gameDesc; (void)secretKey; (void)version; (void)pingRooms; (void)crossPingRooms; return PEERFalse; }
PEERBool peerSetTitle(PEER peer, const char *title, const char *description, const char *gameDesc,
                  const char *secretKey, int version, int pingTimeout, PEERBool flag,
                  PEERBool pingRooms[16], PEERBool crossPingRooms[16])
{ (void)peer; (void)title; (void)description; (void)gameDesc; (void)secretKey; (void)version; (void)pingTimeout; (void)flag; (void)pingRooms; (void)crossPingRooms; return PEERFalse; }

void peerListGroupRooms(PEER peer, peerGroupListCallback callback, void *param, PEERBool fresh)
{ (void)peer; (void)callback; (void)param; (void)fresh; }
void peerListGroupRooms(PEER peer, const char *filter, peerGroupListCallback callback, void *param, PEERBool fresh)
{ (void)peer; (void)filter; (void)callback; (void)param; (void)fresh; }
void peerJoinGroupRoom(PEER peer, int groupID, peerJoinRoomCallback callback, void *param, PEERBool authenticate)
{ (void)peer; (void)groupID; (void)callback; (void)param; (void)authenticate; }
void peerJoinStagingRoom(PEER peer, SBServer server, const char *password, peerJoinRoomCallback callback,
                         void *param, PEERBool authenticate)
{ (void)peer; (void)server; (void)password; (void)callback; (void)param; (void)authenticate; }
void peerCreateStagingRoom(PEER peer, const char *name, int maxPlayers, const char *password,
                           peerJoinRoomCallback callback, void *param, PEERBool authenticate)
{ (void)peer; (void)name; (void)maxPlayers; (void)password; (void)callback; (void)param; (void)authenticate; }
void peerCreateStagingRoomWithSocket(PEER peer, const char *name, int maxPlayers, const char *password,
                                     unsigned int qr2Sock, unsigned short preferredQRPort,
                                     peerJoinRoomCallback callback, void *param, PEERBool authenticate)
{ (void)peer; (void)name; (void)maxPlayers; (void)password; (void)qr2Sock; (void)preferredQRPort; (void)callback; (void)param; (void)authenticate; }
void peerLeaveRoom(PEER peer, RoomType roomType, const char *reason) { (void)peer; (void)roomType; (void)reason; }
void peerMessageRoom(PEER peer, RoomType roomType, const char *message, PEERBool authenticate)
{ (void)peer; (void)roomType; (void)message; (void)authenticate; }
void peerMessagePlayer(PEER peer, const char *nick, const char *message, MessageType messageType)
{ (void)peer; (void)nick; (void)message; (void)messageType; }
void peerUTMRoom(PEER peer, RoomType roomType, const char *key, const char *value, PEERBool authenticate)
{ (void)peer; (void)roomType; (void)key; (void)value; (void)authenticate; }
void peerUTMPlayer(PEER peer, const char *nick, const char *key, const char *value, PEERBool authenticate)
{ (void)peer; (void)nick; (void)key; (void)value; (void)authenticate; }
void peerStartGame(PEER peer, const char *message, int flags) { (void)peer; (void)message; (void)flags; }
void peerStopGame(PEER peer) { (void)peer; }
void peerUpdateGame(PEER peer, SBServer server, PEERBool fullUpdate) { (void)peer; (void)server; (void)fullUpdate; }
void peerSetReady(PEER peer, PEERBool ready) { (void)peer; (void)ready; }
void peerSetGlobalKeys(PEER peer, int num, const char **keys, const char **values) { (void)peer; (void)num; (void)keys; (void)values; }
const char *peerGetGlobalWatchKey(PEER peer, const char *nick, const char *key) { (void)peer; (void)nick; (void)key; return ""; }
void peerSetGlobalWatchKeys(PEER peer, RoomType roomType, int num, const char **keys, PEERBool removeKeys)
{ (void)peer; (void)roomType; (void)num; (void)keys; (void)removeKeys; }
void peerSetRoomWatchKeys(PEER peer, RoomType roomType, int num, const char **keys, PEERBool removeKeys)
{ (void)peer; (void)roomType; (void)num; (void)keys; (void)removeKeys; }
void peerSetRoomKeys(PEER peer, RoomType roomType, const char *nick, int num, const char **keys, const char **values)
{ (void)peer; (void)roomType; (void)nick; (void)num; (void)keys; (void)values; }
void peerGetRoomKeys(PEER peer, RoomType roomType, const char *keyFilter, int numKeys, const char **keys,
                     peerGetRoomKeysCallback callback, void *param, PEERBool onlyCurrentPlayers)
{ (void)peer; (void)roomType; (void)keyFilter; (void)numKeys; (void)keys; (void)callback; (void)param; (void)onlyCurrentPlayers; }
void peerEnumPlayers(PEER peer, RoomType roomType, peerGetInfoCallback callback, void *userData)
{ (void)peer; (void)roomType; (void)callback; (void)userData; }
void peerStartListingGames(PEER peer, const char *filter, peerListingGamesCallback callback, void *userData)
{ (void)peer; (void)filter; (void)callback; (void)userData; }
void peerStartListingGames(PEER peer, const unsigned char *keyIDs, int numKeys, const char *filter,
                           peerListingGamesCallback2 callback, void *userData)
{ (void)peer; (void)keyIDs; (void)numKeys; (void)filter; (void)callback; (void)userData; }
void peerStopListingGames(PEER peer) { (void)peer; }
void peerSetUpdatesRoomChannel(PEER peer, const char *channel) { (void)peer; (void)channel; }
PEERBool peerGetPlayerInfoNoWait(PEER peer, const char *nick, unsigned int *ip, int *profileID)
{ (void)peer; (void)nick; if (ip) *ip = 0; if (profileID) *profileID = 0; return PEERFalse; }
PEERBool peerGetPlayerFlags(PEER peer, const char *nick, RoomType roomType, int *flags)
{ (void)peer; (void)nick; (void)roomType; if (flags) *flags = 0; return PEERFalse; }
void peerGetPlayerProfileID(PEER peer, const char *nick, peerProfileIDCallback callback, int *profileID, PEERBool onlyCurrentPlayers)
{ (void)peer; (void)nick; (void)callback; if (profileID) *profileID = 0; (void)onlyCurrentPlayers; }
void peerAuthenticateCDKey(PEER peer, const char *cdKey, peerAuthenticateCDKeyCallback callback, void *param, PEERBool useAck)
{ (void)peer; (void)cdKey; (void)callback; (void)param; (void)useAck; }
unsigned int peerGetLocalIP(PEER peer) { (void)peer; return 0; }
void peerParseQuery(PEER peer, char *indata, int error, void *addr) { (void)peer; (void)indata; (void)error; (void)addr; }
