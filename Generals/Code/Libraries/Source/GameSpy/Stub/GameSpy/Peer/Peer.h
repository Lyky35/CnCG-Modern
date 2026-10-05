#pragma once

#include <windows.h>

typedef void* PEER;
typedef int PEERBool;
typedef int PEERJoinResult;
typedef int RoomType;

#define PEERTrue 1
#define PEERFalse 0


// Peer join results (GameSpy Peer SDK Peer.h values)
enum {
    PEERJoinSuccess = 0,
    PEERJoinFailed = 50,
    PEERNoConnection = 1,
    PEERAlreadyInRoom = 2,
    PEERBadPassword = 3,
    PEERInviteOnlyRoom = 4,
    PEERFullRoom = 5,
    PEERNameInUse = 6,
    PEERNotResponding = 7,
    PEERBannedFromRoom = 8,
    PEERServerFull = 9,
    PEERGameStarted = 10,
    PEERNotEnoughPlayers = 11,
    PEEROther = 100
};

// Player-list update types
enum {
    PEER_COMPLETE = 0,
    PEER_IN_USE = 999
};

enum {
    PEER_ADD = 1,
    PEER_UPDATE = 2,
    PEER_REMOVE = 3,
    PEER_CLEAR = 4
};

// Player flags bit
#define PEER_FLAG_OP 1


// Room types (GameSpy Peer SDK)
enum {
    UnknownRoom = 0,
    ChatRoom = 1,
    StagingRoom = 2,
    GroupRoom = 3,
    GameRoom = 4,
    TitleRoom = 5,
    NumRooms = 6
};


// Chat message types (GameSpy Chat SDK)
enum MessageType {
    NormalMessage = 0,
    ActionMessage = 1
};

/* ------------------------------------------------------------------------ */
/* QR2 (server browser query) SDK surface - stub declarations               */
/* ------------------------------------------------------------------------ */
typedef void * qr2_buffer_t;
typedef void * qr2_keybuffer_t;

enum qr2_key_type {
    key_server = 0,
    key_player = 1,
    key_team = 2,
    key_invalid = 3,
    QR_KEY_LIST = 4,
    QR_COUNT = 5,
    QR_ADD_ERROR = 6,
    QR_NAT_NEGOTIATE = 7
};

enum qr2_error_t {
    e_qrnoerror = 0,
    e_qrwsockerror = 1,
    e_qrbinderror = 2,
    e_qrdnserror = 3,
    e_qrconnerror = 4
};

#define NUM_RESERVED_KEYS 12

enum {
    HOSTNAME_KEY = 0,
    GAMEVER_KEY,
    GAMENAME_KEY,
    MAPNAME_KEY
};

void qr2_buffer_add(qr2_buffer_t buffer, const char *keyValuePair);
void qr2_buffer_add_int(qr2_buffer_t buffer, int value);
void qr2_keybuffer_add(qr2_keybuffer_t keyBuffer, int key);
void qr2_register_key(int key, const char *name);

/* ------------------------------------------------------------------------ */
/* Chat SDK surface - stub declarations                                      */
/* ------------------------------------------------------------------------ */
enum {
    GE_NOERROR = 0,
    GE_NOTCONNECTED = 1,
    GE_TIMEOUT = 2,
    GE_LOGIN_BAD_NICK = 3,
    GE_LOGIN_BAD_PASSWORD = 4,
    GE_LOGIN_BAD_EMAIL = 5,
    GE_LOGIN_BANNED = 6,
    GE_LOGIN_CONNECTION_FAILED = 7,
    GE_LOGIN_UNIQUE_LOGIN_FAIL = 8,
    GE_LOGIN_GAME_SNAP_FAIL = 9
};

#define SNAP_FINAL 1
#define SNAP_NORMAL 0

char *GetChallenge(void *session);
int GenerateAuth(const char *challenge, char *password, char *outAuth);
int NewGame(unsigned int maxPlayers);
int SendGameSnapShot(void *handle, const char *snapshot, int finalFlag);
int FreeGame(void *handle);
void chatSetLocalIP(unsigned int localIP);

extern char gcd_gamename[128];
extern char gcd_secret_key[128];

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
typedef void (*peerDisconnectedCallback)(PEER peer, const char *reason, void *param);
typedef void (*peerReadyChangedCallback2)(PEER peer, const char *nick, PEERBool ready, void *param);
typedef void (*peerRoomMessageCallback)(PEER peer, RoomType roomType, const char *nick, const char *message, MessageType messageType, void *param);
typedef void (*peerPlayerMessageCallback)(PEER peer, const char *nick, const char *message, MessageType messageType, void *param);
typedef void (*peerGameStartedCallback2)(PEER peer, unsigned int IP, const char *message, void *param);
typedef void (*peerPlayerJoinedCallback)(PEER peer, RoomType roomType, const char *nick, void *param);
typedef void (*peerPlayerLeftCallback)(PEER peer, RoomType roomType, const char *nick, const char *reason, void *param);
typedef void (*peerPlayerChangedNickCallback)(PEER peer, RoomType roomType, const char *oldNick, const char *newNick, void *param);
typedef void (*peerPingCallback)(PEER peer, const char *nick, int ping, void *param);
typedef void (*peerCrossPingCallback)(PEER peer, const char *nick1, const char *nick2, int crossPing, void *param);
typedef void (*peerRoomUTMCallback)(PEER peer, RoomType roomType, const char *nick, const char *command,
                                    const char *parameters, PEERBool authenticated, void *param);
typedef void (*peerPlayerUTMCallback)(PEER peer, const char *nick, const char *command, const char *parameters,
                                      PEERBool authenticated, void *param);
typedef void (*peerKeyChangedCallback2)(PEER peer, const char *nick, const char *key, const char *val, void *param);
typedef void (*peerRoomKeyChangedCallback)(PEER peer, RoomType roomType, const char *nick, const char *key, const char *val, void *param);
typedef void (*peerQRServerKeyCallback)(PEER peer, int key, qr2_buffer_t buffer, void *param);
typedef void (*peerQRPlayerKeyCallback)(PEER peer, int key, int index, qr2_buffer_t buffer, void *param);
typedef void (*peerQRTeamKeyCallback)(PEER peer, int key, int index, qr2_buffer_t buffer, void *param);
typedef void (*peerQRKeyListCallback)(PEER peer, qr2_key_type type, qr2_keybuffer_t keyBuffer, void *param);
typedef int (*peerQRCountCallback)(PEER peer, qr2_key_type type, void *param);
typedef void (*peerQRAddErrorCallback)(PEER peer, qr2_error_t error, char *errorString, void *param);
typedef void (*peerQRNatNegotiateCallback)(PEER peer, int cookie, void *param);
typedef void (*peerKickedCallback)(PEER peer, RoomType roomType, const char *nick, const char *reason, void *param);
typedef void (*peerPlayerInfoCallback)(PEER peer, RoomType roomType, const char *nick, unsigned int IP, int profileID, void *param);
typedef void (*peerPlayerFlagsChangedCallback)(PEER peer, RoomType roomType, const char *nick, int oldFlags, int newFlags, void *param);
typedef void (*peerNewPlayerListCallback)(PEER peer, RoomType roomType, void *param);
typedef void (*peerGOACallback)(PEER peer, PEERBool playing, char *outbuf, int maxlen, void *param);
typedef void (*peerKeyChangedCallback)(PEER peer, const char *key, const char *value, void *param);
typedef void (*peerEnumPlayersCallback)(PEER peer, PEERBool success, RoomType roomType, int index, const char *nick, int flags, void *param);
typedef void (*peerListingGamesCallback)(PEER peer, PEERBool success, int gameID, void *param);

typedef struct {
    peerDisconnectedCallback disconnected;
    peerErrorCallback error;
    peerNickErrorCallback nickError;
    peerConnectCallback connect;
    peerJoinRoomCallback joinRoom;
    peerRoomMessageCallback roomMessage;
    peerPlayerMessageCallback playerMessage;
    peerPlayerJoinedCallback playerJoined;
    peerPlayerLeftCallback playerLeft;
    peerPlayerChangedNickCallback playerChangedNick;
    peerReadyChangedCallback2 readyChanged;
    peerPingCallback ping;
    peerCrossPingCallback crossPing;
    peerRoomUTMCallback roomUTM;
    peerPlayerUTMCallback playerUTM;
    peerKeyChangedCallback2 globalKeyChanged;
    peerRoomKeyChangedCallback roomKeyChanged;
    peerQRServerKeyCallback qrServerKey;
    peerQRPlayerKeyCallback qrPlayerKey;
    peerQRTeamKeyCallback qrTeamKey;
    peerQRKeyListCallback qrKeyList;
    peerQRCountCallback qrCount;
    peerQRAddErrorCallback qrAddError;
    peerQRNatNegotiateCallback qrNatNegotiateCallback;
    peerKickedCallback kicked;
    peerPlayerInfoCallback playerInfo;
    peerPlayerFlagsChangedCallback playerFlagsChanged;
    peerNewPlayerListCallback newPlayerList;
    peerGameStartedCallback2 gameStarted;
    peerGOACallback GOABasic;
    peerGOACallback GOAInfo;
    peerGOACallback GOARules;
    peerGOACallback GOAPlayers;
    void *param;
} PEERCallbacks;



struct _SBServer { void *keyvals; };
typedef struct _SBServer * SBServer;

typedef void (*peerGroupListCallback)(PEER peer, PEERBool success, int groupID, SBServer server,
                                      const char *name, int numWaiting, int maxWaiting,
                                      int numGames, int numPlaying, void *param);
typedef void (*peerListingGamesCallback2)(PEER peer, PEERBool success, const char *name, SBServer server,
                                          PEERBool staging, int msg, int percentListed, void *param);
typedef void (*peerGetRoomKeysCallback)(PEER peer, PEERBool success, RoomType roomType, const char *nick,
                                        int num, char **keys, char **values, void *param);
typedef void (*peerProfileIDCallback)(PEER peer, PEERBool success, const char *nick, int profileID, void *param);
typedef void (*peerAuthenticateCDKeyCallback)(PEER peer, int result, void *param);
typedef void (*peerGetInfoCallback)(PEER peer, PEERBool success, RoomType roomType, int index,
                                    const char *nick, int flags, void *param);

PEER peerInitialize(PEERCallbacks *callbacks);
void peerShutdown(PEER peer);
PEERBool peerConnect(PEER peer, const char *nick, int profileID, peerNickErrorCallback nickCallback,
                     peerConnectCallback connectCallback, void *param, PEERBool useGP);
void peerRetryWithNick(PEER peer, const char *nick);
void peerDisconnect(PEER peer);
void peerThink(PEER peer);
PEERBool peerIsConnected(PEER peer);
void peerStateChanged(PEER peer);
int peerStateChange(void);
PEERBool peerSetTitle(PEER peer, const char *title, const char *description, const char *gameDesc,
                  const char *secretKey, int version, PEERBool pingRooms[16], PEERBool crossPingRooms[16]);
PEERBool peerSetTitle(PEER peer, const char *title, const char *description, const char *gameDesc,
                  const char *secretKey, int version, int pingTimeout, PEERBool flag,
                  PEERBool pingRooms[16], PEERBool crossPingRooms[16]);
void peerListGroupRooms(PEER peer, peerGroupListCallback callback, void *param, PEERBool fresh);
void peerListGroupRooms(PEER peer, const char *filter, peerGroupListCallback callback, void *param, PEERBool fresh);
void peerJoinGroupRoom(PEER peer, int groupID, peerJoinRoomCallback callback, void *param, PEERBool authenticate);
void peerJoinStagingRoom(PEER peer, SBServer server, const char *password, peerJoinRoomCallback callback,
                         void *param, PEERBool authenticate);
void peerCreateStagingRoom(PEER peer, const char *name, int maxPlayers, const char *password,
                           peerJoinRoomCallback callback, void *param, PEERBool authenticate);
void peerCreateStagingRoomWithSocket(PEER peer, const char *name, int maxPlayers, const char *password,
                                     unsigned int qr2Sock, unsigned short preferredQRPort,
                                     peerJoinRoomCallback callback, void *param, PEERBool authenticate);
void peerLeaveRoom(PEER peer, RoomType roomType, const char *reason);
void peerMessageRoom(PEER peer, RoomType roomType, const char *message, PEERBool authenticate);
void peerMessagePlayer(PEER peer, const char *nick, const char *message, MessageType messageType);
void peerUTMRoom(PEER peer, RoomType roomType, const char *key, const char *value, PEERBool authenticate);
void peerUTMPlayer(PEER peer, const char *nick, const char *key, const char *value, PEERBool authenticate);
void peerStartGame(PEER peer, const char *message, int flags);
void peerStopGame(PEER peer);
void peerUpdateGame(PEER peer, SBServer server, PEERBool fullUpdate);
void peerSetReady(PEER peer, PEERBool ready);
void peerSetGlobalKeys(PEER peer, int num, const char **keys, const char **values);
const char *peerGetGlobalWatchKey(PEER peer, const char *nick, const char *key);
void peerSetGlobalWatchKeys(PEER peer, RoomType roomType, int num, const char **keys, PEERBool removeKeys);
void peerSetRoomWatchKeys(PEER peer, RoomType roomType, int num, const char **keys, PEERBool removeKeys);
void peerSetRoomKeys(PEER peer, RoomType roomType, const char *nick, int num, const char **keys, const char **values);
void peerGetRoomKeys(PEER peer, RoomType roomType, const char *keyFilter, int numKeys, const char **keys,
                     peerGetRoomKeysCallback callback, void *param, PEERBool onlyCurrentPlayers);
void peerEnumPlayers(PEER peer, RoomType roomType, peerGetInfoCallback callback, void *userData);
void peerStartListingGames(PEER peer, const char *filter, peerListingGamesCallback callback, void *userData);
void peerStartListingGames(PEER peer, const unsigned char *keyIDs, int numKeys, const char *filter,
                           peerListingGamesCallback2 callback, void *userData);
void peerStopListingGames(PEER peer);
void peerSetUpdatesRoomChannel(PEER peer, const char *channel);
PEERBool peerGetPlayerInfoNoWait(PEER peer, const char *nick, unsigned int *ip, int *profileID);
PEERBool peerGetPlayerFlags(PEER peer, const char *nick, RoomType roomType, int *flags);
void peerGetPlayerProfileID(PEER peer, const char *nick, peerProfileIDCallback callback, int *profileID, PEERBool onlyCurrentPlayers);
void peerAuthenticateCDKey(PEER peer, const char *cdKey, peerAuthenticateCDKeyCallback callback, void *param, PEERBool useAck);
unsigned int peerGetLocalIP(PEER peer);
void peerParseQuery(PEER peer, char *indata, int error, void *addr);
