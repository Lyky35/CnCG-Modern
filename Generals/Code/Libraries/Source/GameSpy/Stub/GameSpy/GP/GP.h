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
    GP_BUSY = 6
};

enum {
    GP_ERROR = 0,
    GP_RECV_BUDDY_REQUEST = 1,
    GP_RECV_BUDDY_MESSAGE = 2,
    GP_RECV_BUDDY_STATUS = 3,
    GP_RECV_BUDDY_UTM = 4,
    GP_BUDDY_STATUS = 5
};

typedef void (*GPErrorCallback)(GPConnection *conn, GPResult result, void *param);
typedef void (*GPRecvBuddyRequestCallback)(GPConnection *conn, GPProfile profile, const char *message, void *param);
typedef void (*GPRecvBuddyMessageCallback)(GPConnection *conn, GPProfile profile, const char *message, void *param);
typedef void (*GPRecvBuddyStatusCallback)(GPConnection *conn, GPProfile profile, GPEnum status, const char *statusString, const char *locationString, void *param);
typedef void (*GPRecvBuddyUTMCallback)(GPConnection *conn, GPProfile profile, const char *key, const char *value, void *param);
typedef void (*GPBuddyStatusCallback)(GPConnection *conn, GPProfile profile, GPEnum status, const char *statusString, const char *locationString, void *param);

GPResult gpInitialize(GPConnection *conn, int productID);
GPResult gpDestroy(GPConnection *conn);
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password, void *param);
GPResult gpConnectNewUser(GPConnection *conn, const char *nick, const char *email, const char *password, void *param);
GPResult gpDisconnect(GPConnection *conn);
GPResult gpSetCallback(GPConnection *conn, int event, void *callback, void *param);
GPResult gpProcess(GPConnection *conn);
GPResult gpSetStatus(GPConnection *conn, GPEnum status, const char *statusString, const char *locationString);
GPResult gpSendBuddyMessage(GPConnection *conn, int recipient, const char *message);
GPResult gpSendBuddyRequest(GPConnection *conn, int id, const char *message);
GPResult gpGetInfo(GPConnection *conn, int profile, void *param);
