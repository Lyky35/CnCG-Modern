#pragma once

#include <windows.h>

typedef void* GPConnection;
typedef int GPProfile;
typedef int GPResult;
typedef int GPErrorCode;
typedef int GPEnum;

#define GP_NICK_LEN 31
#define GP_EMAIL_LEN 51
#define GP_PASSWORD_LEN 31
#define GP_STATUS_STRING_LEN 256
#define GP_LOCATION_STRING_LEN 256
#define GP_COUNTRYCODE_LEN 3
#define GP_REASON_LEN 1024

enum {
    GP_OFFLINE = 0,
    GP_ONLINE = 1,
    GP_CHATTING = 2,
    GP_AWAY = 3,
    GP_STAGING = 4,
    GP_PLAYING = 5,
    GP_BUSY = 6,
    GP_RECV_GAME_INVITE = 7
};



// Legacy GP handle alias used by the game's chat layer
typedef void * GServer;

// Callback argument structures (GameSpy GP SDK GP.h)
typedef struct { GPProfile profile; const char *message; const char *reason; } GPRecvBuddyRequestArg;
typedef struct { GPProfile profile; const char *message; const char *date; const char *reason; } GPRecvBuddyMessageArg;
typedef struct { GPProfile profile; GPEnum status; const char *statusString; const char *locationString; int index; } GPRecvBuddyStatusArg;
typedef struct { GPResult result; int errorCode; const char *errorString; int fatal; } GPErrorArg;
typedef struct { GPResult result; int index; GPProfile profile; const char *nick; const char *email; const char *countrycode; const char *date; const char *reason; } GPConnectResponseArg;

// GPCallback: single-member union so function pointers can be cast through it
typedef void (*GPCallback)(void);

typedef struct {
    GPProfile profile;
    GPEnum status;
    const char *statusString;
    const char *locationString;
    char *nick;
    char *first;
    char *mi;
    char *last;
    char *age;
    char *sex;
    char *homepage;
    char *location;
    char *email;
    char *bio;
    char *interests;
    char *occupation;
    char *city;
    char *state;
    char *country;
    char *countrycode;
} GPStatus;

// GP result codes / flags / events (GameSpy GP SDK)
enum {
    GP_NO_ERROR = 0,
    GP_INTERNAL_ERROR = 1,
    GP_SERVER_ERROR = 2,
    GP_TIMEOUT = 3,
    GP_MEMORY_ERROR = 4,
    GP_NETWORK_ERROR = 5,
    GP_NOT_CONNECTED = 6,
    GP_NOT_LOGGED_IN = 7,
    GP_FILE_NOT_FOUND = 8,
    GP_FILE_TOO_BIG = 9,
    GP_FILE_NO_SPACE = 10,
    GP_FILE_FAILED = 11,
    GP_INVALID_INPUT = 12,
    GP_UNKNOWN_TYPE = 13,
    GP_FAILED = 14,
    GP_SERVER_NOT_RESPONDING = 15,
    GP_NO_SERVERS = 16,
    GP_PROTOCOL_ERROR = 17,
    GP_SERVER_FULL = 18,
    GP_VERSION_MISMATCH = 19,
    GP_INVALID_LOGIN = 20,
    GP_INVALID_QUERY_RESULT_TYPE = 21,
    GP_SERVER_ERROR_ = 22,
    GP_PARAMETER_ERROR = 23,
    GP_CONNECTION_CLOSED = 24,
    GP_FORCED_DISCONNECT = 25,

    GP_NO_FIREWALL = 0x100,
    GP_FIREWALL = 0x101,
    GP_BLOCKING = 0x200,
    GP_NON_BLOCKING = 0x201,
    GP_CHECK_CACHE = 0x203,
    GP_DONT_CHECK_CACHE = 0x204,
    GP_READ_FROM_FILE = 0x205,
    GP_WRITE_TO_FILE = 0x206,
    GP_PARSE = 0x300,
    GP_GENERAL = 0x301,
    GP_NETWORK = 0x302,
    GP_DATABASE = 0x303,

    GP_LOGIN = 0x400,
    GP_LOGIN_BAD_EMAIL = 0x401,
    GP_LOGIN_BAD_NICK = 0x402,
    GP_LOGIN_BAD_PASSWORD = 0x403,
    GP_LOGIN_BAD_PROFILE = 0x404,
    GP_LOGIN_CONNECTION_FAILED = 0x405,
    GP_LOGIN_PROFILE_DELETED = 0x406,
    GP_LOGIN_SERVER_AUTH_FAILED = 0x407,
    GP_LOGIN_TIMEOUT = 0x408,
    GP_NEWUSER = 0x410,
    GP_NEWUSER_BAD_NICK = 0x411,
    GP_NEWUSER_BAD_PASSWORD = 0x412,
    GP_NEWPROFILE = 0x420,
    GP_NEWPROFILE_BAD_NICK = 0x421,
    GP_NEWPROFILE_BAD_OLD_NICK = 0x422,
    GP_UPDATEPRO = 0x430,
    GP_UPDATEPRO_BAD_NICK = 0x431,
    GP_UPDATEUI = 0x440,
    GP_UPDATEUI_BAD_EMAIL = 0x441,
    GP_ADDBUDDY = 0x450,
    GP_ADDBUDDY_ALREADY_BUDDY = 0x451,
    GP_ADDBUDDY_BAD_FROM = 0x452,
    GP_ADDBUDDY_BAD_NEW = 0x453,
    GP_DELBUDDY = 0x460,
    GP_DELBUDDY_NOT_BUDDY = 0x461,
    GP_AUTHADD = 0x470,
    GP_AUTHADD_BAD_FROM = 0x471,
    GP_AUTHADD_BAD_SIG = 0x472,
    GP_BM = 0x480,
    GP_BM_NOT_BUDDY = 0x481,
    GP_GETPROFILE = 0x490,
    GP_GETPROFILE_BAD_PROFILE = 0x491,
    GP_SEARCH = 0x4A0,
    GP_SEARCH_CONNECTION_FAILED = 0x4A1,
    GP_DELPROFILE = 0x4B0,
    GP_DELPROFILE_LAST_PROFILE = 0x4B1,
    GP_BAD_SESSKEY = 0x4C0,
    GP_CONNECTED = 0x4D0,
    GP_ERROR = 0x500,
    GP_FATAL = 0x501,
    GP_ONCONNECT = 0x502,
    GP_STATUS = 0x503,
    GP_RECV_BUDDY_STATUS = 0x510,
    GP_RECV_BUDDY_MESSAGE = 0x511,
    GP_RECV_BUDDY_REQUEST = 0x512,
    GP_RECV_BUDDY_UTM = 0x513,

    GP_MASK_NONE = 0x600,
    GP_MASK_PUBLIC = 0x601
};

GPResult gpInitialize(GPConnection *conn, int productID);
GPResult gpDestroy(GPConnection *conn);
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password, void *param);
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password,
                  GPCallback connectResponse, GPCallback newUserResponse, GPCallback errorResponse, void *param);
GPResult gpConnectNewUser(GPConnection *conn, const char *nick, const char *email, const char *password, void *param);
GPResult gpConnectNewUser(GPConnection *conn, const char *nick, const char *email, const char *password,
                          GPEnum flags, GPEnum blocking, GPCallback callback, void *param);
GPResult gpDeleteProfile(GPConnection *conn);
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password,
                  GPEnum connectFlags, GPEnum blocking, GPCallback callback, GPResult *outError);
GPResult gpDisconnect(GPConnection *conn);
GPResult gpSetCallback(GPConnection *conn, int event, GPCallback callback, void *param);
GPResult gpProcess(GPConnection *conn);
GPResult gpSetStatus(GPConnection *conn, GPEnum status, const char *statusString, const char *locationString);
GPResult gpSendBuddyMessage(GPConnection *conn, int recipient, const char *message);
GPResult gpSendBuddyRequest(GPConnection *conn, int id, const char *message);
GPResult gpGetInfo(GPConnection *conn, int profile, GPCallback callback, void *param);
GPResult gpGetInfo(GPConnection *conn, int profile, GPEnum checkCache, GPEnum blocking, GPCallback callback, void *param);
GPResult gpGetInfoFull(GPConnection *conn, int profile, GPCallback callback, void *param);
GPResult gpGetBuddyStatus(GPConnection *conn, int buddyIndex, GPStatus *status);
GPResult gpGetBuddyIndex(GPConnection *conn, GPProfile profile, int *buddyIndex);
GPResult gpSetInfoMask(GPConnection *conn, int mask);
GPResult gpIsConnected(GPConnection *conn, int *outConnected);
GPResult gpDeleteProfile(GPConnection *conn, int flags);
GPResult gpDeleteBuddy(GPConnection *conn, GPProfile profile);
GPResult gpAuthBuddyRequest(GPConnection *conn, GPProfile profile);
GPResult gpDenyBuddyRequest(GPConnection *conn, GPProfile profile);
typedef GPStatus GPBuddyStatus;
typedef struct {
    GPResult result;
    GPProfile profile;
    char *nick;
    char *first;
    char *mi;
    char *last;
    char *age;
    char *sex;
    char *homepage;
    char *location;
    char *email;
    char *bio;
    char *interests;
    char *occupation;
    char *city;
    char *state;
    char *country;
    char *countrycode;
    char *reason;
    char *date;
} GPGetInfoResponseArg;

char *gpGetValueString(GPStatus *status, const char *property);
int gpValueExists(GPStatus *status, const char *property);

