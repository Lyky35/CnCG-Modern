#include "GameSpy/Peer/Peer.h"

PEER peerInitialize(PEERCallbacks *callbacks) { return NULL; }
void peerShutdown(PEER peer) {}
PEERBool peerConnect(PEER peer, const char *nick, int profileID, void *param) { return PEERFalse; }
void peerRetryWithNick(PEER peer, const char *nick) {}
void peerThink(PEER peer) {}
PEERBool peerIsConnected(PEER peer) { return PEERFalse; }
void peerSetTitle(PEER peer, const char *title, const char *secretKey, void *param) {}
void peerListGroupRooms(PEER peer, void *param) {}
void peerJoinGroupRoom(PEER peer, int groupID, void *param, PEERBool authenticate) {}
void peerJoinStagingRoom(PEER peer, const char *server, const char *password, void *param, PEERBool authenticate) {}
void peerCreateStagingRoom(PEER peer, const char *name, int maxPlayers, const char *password, void *param, PEERBool authenticate) {}
void peerCreateStagingRoomWithSocket(PEER peer, const char *name, int maxPlayers, const char *password, void *param, PEERBool authenticate) {}
void peerLeaveRoom(PEER peer, RoomType roomType, const char *reason) {}
void peerMessageRoom(PEER peer, RoomType roomType, const char *message, PEERBool authenticate) {}
void peerMessagePlayer(PEER peer, const char *nick, const char *message, PEERBool authenticate) {}
void peerUTMRoom(PEER peer, RoomType roomType, const char *key, const char *value, PEERBool authenticate) {}
void peerUTMPlayer(PEER peer, const char *nick, const char *key, const char *value, PEERBool authenticate) {}
void peerStartGame(PEER peer, const char *message, int flags) {}
void peerSetReady(PEER peer, PEERBool ready) {}
void peerEnumPlayers(PEER peer, RoomType roomType, peerEnumPlayersCallback callback, void *userData) {}
void peerStartListingGames(PEER peer, const char *filter, peerListingGamesCallback callback, void *userData) {}
void peerStopListingGames(PEER peer) {}
void peerSetUpdatesRoomChannel(PEER peer, const char *channel) {}
PEERBool peerGetPlayerInfoNoWait(PEER peer, const char *nick, unsigned int *ip, int *id) { return PEERFalse; }
PEERBool peerGetPlayerFlags(PEER peer, const char *nick, RoomType roomType, int *flags) { return PEERFalse; }
unsigned int peerGetLocalIP(PEER peer) { return 0; }
