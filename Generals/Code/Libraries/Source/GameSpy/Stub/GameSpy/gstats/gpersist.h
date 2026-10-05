#pragma once

#include <windows.h>

typedef void (*PersistDataCallback)(int localid, int profileid, int type, int index, void *data, void *param);

int InitStatsConnection(int port);
int IsStatsConnected(void);
void CloseStatsConnection(void);
void PersistThink(void);
int GetPersistDataValues(int localid, int profileid, int type, int index, const char *keys, PersistDataCallback callback, void *param);
int SetPersistDataValues(int localid, int profileid, int type, int index, const char *data, PersistDataCallback callback, void *param);
