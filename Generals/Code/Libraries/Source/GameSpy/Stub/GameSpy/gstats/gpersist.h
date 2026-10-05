#pragma once

#include <windows.h>
#include "GameSpy/gstats/gserver.h"

typedef enum {
    pd_public_ro = 0,
    pd_public_rw,
    pd_private_ro,
    pd_private_rw
} persisttype_t;

typedef void (*PersistDataCallback)(int localid, int profileid, persisttype_t type, int index, char *data, void *param);
typedef void (*PersistSetCallback)(int localid, int profileid, persisttype_t type, int index, int success, void *param);
typedef void (*GPSPreAuthCallback)(int localid, int profileid, int authenticated, char *errmsg, void *param);

int SetPersistDataValues(int localid, int profileid, persisttype_t type, int index, const char *keysValues,
                         PersistSetCallback callback, void *param);
int PreAuthenticatePlayerPM(int localid, int profileid, char *authToken, GPSPreAuthCallback callback, void *param);
int PreAuthenticatePlayerCD(int localid, const char *cdkey, char *cdkeyHash, char *authToken, GPSPreAuthCallback callback, void *param);

int InitStatsConnection(int port);
int IsStatsConnected(void);
void CloseStatsConnection(void);
void PersistThink(void);
int GetPersistDataValues(int localid, int profileid, int type, int index, const char *keys, PersistDataCallback callback, void *param);
int SetPersistDataValues(int localid, int profileid, int type, int index, const char *data, PersistDataCallback callback, void *param);
