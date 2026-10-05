#include "GameSpy/GP/GP.h"

GPResult gpInitialize(GPConnection *conn, int productID) { return 0; }
GPResult gpDestroy(GPConnection *conn) { return 0; }
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password, void *param) { return 0; }
GPResult gpConnectNewUser(GPConnection *conn, const char *nick, const char *email, const char *password, void *param) { return 0; }
GPResult gpDisconnect(GPConnection *conn) { return 0; }
GPResult gpSetCallback(GPConnection *conn, int event, void *callback, void *param) { return 0; }
GPResult gpProcess(GPConnection *conn) { return 0; }
GPResult gpSetStatus(GPConnection *conn, GPEnum status, const char *statusString, const char *locationString) { return 0; }
GPResult gpSendBuddyMessage(GPConnection *conn, int recipient, const char *message) { return 0; }
GPResult gpSendBuddyRequest(GPConnection *conn, int id, const char *message) { return 0; }
GPResult gpGetInfo(GPConnection *conn, int profile, void *param) { return 0; }
