#pragma once

#include <windows.h>

typedef void* PEER;
typedef int PEERBool;
typedef int PEERJoinResult;
typedef int RoomType;

#define PEERTrue 1
#define PEERFalse 0

enum {
    PEER_STOP_REPORTING = 0,
    PEER_KEEP_REPORTING = 1
};

enum {
    PEER_ROOM = 0,
    PEER_STAGINGROOM = 1,
    PEER_CHATROOM = 2
};

typedef void (*peerConnectCallback)(PEER peer, PEERBool success, void *param);
typedef void (*peerErrorCallback)(PEER peer, const char *error, void *param);
typedef void (*peerNickErrorCallback)(PEER peer, int type, const char *nick, void *param);
typedef void (*peerJoinRoomCallback)(PEER peer, PEERBool success, PEERJoinResult result, RoomType roomType, void *param);
typedef void (*peerMessageRoomCallback)(PEER peer, RoomType roomType, const char *nick, const char *message, void *param);
typedef void (*peerMessagePlayerCallback)(PEER peer, const char *nick, const char *message, void *param);
typedef void (*peerPlayerJoinedCallback)(PEER peer, RoomType roomType, const char *nick, void *param);
typedef void (*peerPlayerLeftCallback)(PEER peer, RoomType roomType, const char *nick, const char *reason, void *param);
typedef void (*peerPlayerChangedNickCallback)(PEER peer, RoomType roomType, const char *oldNick, const char *newNick, void *param);
typedef void (*peerReadyChangedCallback)(PEER peer, const char *nick, PEERBool ready, void *param);
typedef void (*peerPingCallback)(PEER peer, const char *nick, int ping, void *param);
typedef void (*peerCrossPingCallback)(PEER peer, const char *nick1, const char *nick2, int crossPing, void *param);
typedef void (*peerUTMReceivedCallback)(PEER peer, RoomType roomType, const char *nick, const char *key, const char *value, void *param);
typedef void (*peerGameStartedCallback)(PEER peer, unsigned int IP, void *param);
typedef void (*peerListingGamesCallback)(PEER peer, PEERBool success, int gameID, void *param);
typedef void (*peerEnumPlayersCallback)(PEER peer, PEERBool success, RoomType roomType, int index, const char *nick, int flags, void *param);
typedef void (*peerGOABasicCallback)(PEER peer, PEERBool playing, char *outbuf, void *param);
typedef void (*peerGOAInfoCallback)(PEER peer, PEERBool playing, char *outbuf, void *param);
typedef void (*peerGOARulesCallback)(PEER peer, PEERBool playing, char *outbuf, void *param);
typedef void (*peerGOAPlayersCallback)(PEER peer, PEERBool playing, char *outbuf, void *param);

typedef struct {
    peerConnectCallback connectCallback;
    peerErrorCallback errorCallback;
    peerNickErrorCallback nickErrorCallback;
    peerJoinRoomCallback joinRoomCallback;
    peerMessageRoomCallback messageRoomCallback;
    peerMessagePlayerCallback messagePlayerCallback;
    peerPlayerJoinedCallback playerJoinedCallback;
    peerPlayerLeftCallback playerLeftCallback;
    peerPlayerChangedNickCallback playerChangedNickCallback;
    peerReadyChangedCallback readyChangedCallback;
    peerPingCallback pingCallback;
    peerCrossPingCallback crossPingCallback;
    peerUTMReceivedCallback utmReceivedCallback;
    peerGameStartedCallback gameStartedCallback;
} PEERCallbacks;

PEER peerInitialize(PEERCallbacks *callbacks);
void peerShutdown(PEER peer);
PEERBool peerConnect(PEER peer, const char *nick, int profileID, void *param);
void peerRetryWithNick(PEER peer, const char *nick);
void peerThink(PEER peer);
PEERBool peerIsConnected(PEER peer);
void peerSetTitle(PEER peer, const char *title, const char *secretKey, void *param);
void peerListGroupRooms(PEER peer, void *param);
void peerJoinGroupRoom(PEER peer, int groupID, void *param, PEERBool authenticate);
void peerJoinStagingRoom(PEER peer, const char *server, const char *password, void *param, PEERBool authenticate);
void peerCreateStagingRoom(PEER peer, const char *name, int maxPlayers, const char *password, void *param, PEERBool authenticate);
void peerCreateStagingRoomWithSocket(PEER peer, const char *name, int maxPlayers, const char *password, void *param, PEERBool authenticate);
void peerLeaveRoom(PEER peer, RoomType roomType, const char *reason);
void peerMessageRoom(PEER peer, RoomType roomType, const char *message, PEERBool authenticate);
void peerMessagePlayer(PEER peer, const char *nick, const char *message, PEERBool authenticate);
void peerUTMRoom(PEER peer, RoomType roomType, const char *key, const char *value, PEERBool authenticate);
void peerUTMPlayer(PEER peer, const char *nick, const char *key, const char *value, PEERBool authenticate);
void peerStartGame(PEER peer, const char *message, int flags);
void peerSetReady(PEER peer, PEERBool ready);
void peerEnumPlayers(PEER peer, RoomType roomType, peerEnumPlayersCallback callback, void *userData);
void peerStartListingGames(PEER peer, const char *filter, peerListingGamesCallback callback, void *userData);
void peerStopListingGames(PEER peer);
void peerSetUpdatesRoomChannel(PEER peer, const char *channel);
PEERBool peerGetPlayerInfoNoWait(PEER peer, const char *nick, unsigned int *ip, int *id);
PEERBool peerGetPlayerFlags(PEER peer, const char *nick, RoomType roomType, int *flags);
unsigned int peerGetLocalIP(PEER peer);
