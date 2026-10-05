#include "GameSpy/gstats/gpersist.h"

int InitStatsConnection(int port) { return 0; }
int IsStatsConnected(void) { return 0; }
void CloseStatsConnection(void) {}
void PersistThink(void) {}
int GetPersistDataValues(int localid, int profileid, int type, int index, const char *keys, PersistDataCallback callback, void *param) { return 0; }
int SetPersistDataValues(int localid, int profileid, int type, int index, const char *data, PersistDataCallback callback, void *param) { return 0; }
