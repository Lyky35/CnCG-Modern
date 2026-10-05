/*
**	No-op implementations for the GameSpy GP SDK surface declared in the stub
**	GameSpy/GP/GP.h. Buddy-list multiplayer services are unavailable in this
**	build; these definitions exist so the engine links and calls degrade.
*/

#include "GameSpy/GP/GP.h"
#include <string.h>

GPResult gpInitialize(GPConnection *conn, int productID) { (void)conn; (void)productID; return GP_NO_ERROR; }
GPResult gpDestroy(GPConnection *conn) { (void)conn; return GP_NO_ERROR; }
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password, void *param)
{ (void)conn; (void)nick; (void)email; (void)password; (void)param; return GP_NO_ERROR; }
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password,
                  GPCallback connectResponse, GPCallback newUserResponse, GPCallback errorResponse, void *param)
{ (void)conn; (void)nick; (void)email; (void)password; (void)connectResponse; (void)newUserResponse; (void)errorResponse; (void)param; return GP_NO_ERROR; }
GPResult gpConnectNewUser(GPConnection *conn, const char *nick, const char *email, const char *password, void *param)
{ (void)conn; (void)nick; (void)email; (void)password; (void)param; return GP_NO_ERROR; }
GPResult gpDisconnect(GPConnection *conn) { (void)conn; return GP_NO_ERROR; }
GPResult gpSetCallback(GPConnection *conn, int event, GPCallback callback, void *param)
{ (void)conn; (void)event; (void)callback; (void)param; return GP_NO_ERROR; }
GPResult gpProcess(GPConnection *conn) { (void)conn; return GP_NO_ERROR; }
GPResult gpConnect(GPConnection *conn, const char *nick, const char *email, const char *password,
                  GPEnum connectFlags, GPEnum blocking, GPCallback callback, GPResult *outError)
{ (void)conn; (void)nick; (void)email; (void)password; (void)connectFlags; (void)blocking; (void)callback;
  if (outError) *outError = GP_INTERNAL_ERROR;
  return GP_NO_ERROR; }
GPResult gpSetStatus(GPConnection *conn, GPEnum status, const char *statusString, const char *locationString)
{ (void)conn; (void)status; (void)statusString; (void)locationString; return GP_NO_ERROR; }
GPResult gpSendBuddyMessage(GPConnection *conn, int recipient, const char *message)
{ (void)conn; (void)recipient; (void)message; return GP_NO_ERROR; }
GPResult gpSendBuddyRequest(GPConnection *conn, int id, const char *message)
{ (void)conn; (void)id; (void)message; return GP_NO_ERROR; }
GPResult gpGetInfo(GPConnection *conn, int profile, GPCallback callback, void *param)
{ (void)conn; (void)profile; (void)callback; (void)param; return GP_NO_ERROR; }
GPResult gpGetInfo(GPConnection *conn, int profile, GPEnum checkCache, GPEnum blocking, GPCallback callback, void *param)
{ (void)conn; (void)profile; (void)checkCache; (void)blocking; (void)callback; (void)param; return GP_NO_ERROR; }
GPResult gpGetInfoFull(GPConnection *conn, int profile, GPCallback callback, void *param)
{ (void)conn; (void)profile; (void)callback; (void)param; return GP_NO_ERROR; }
GPResult gpConnectNewUser(GPConnection *conn, const char *nick, const char *email, const char *password,
                          GPEnum flags, GPEnum blocking, GPCallback callback, void *param)
{ (void)conn; (void)nick; (void)email; (void)password; (void)flags; (void)blocking; (void)callback; (void)param; return GP_NO_ERROR; }
GPResult gpDeleteProfile(GPConnection *conn) { (void)conn; return GP_NO_ERROR; }
GPResult gpGetBuddyStatus(GPConnection *conn, int buddyIndex, GPStatus *status)
{ (void)conn; (void)buddyIndex; if (status) memset(status, 0, sizeof(*status)); return GP_NO_ERROR; }
GPResult gpGetBuddyIndex(GPConnection *conn, GPProfile profile, int *buddyIndex)
{ (void)conn; (void)profile; if (buddyIndex) *buddyIndex = -1; return GP_NO_ERROR; }
GPResult gpSetInfoMask(GPConnection *conn, int mask) { (void)conn; (void)mask; return GP_NO_ERROR; }
GPResult gpIsConnected(GPConnection *conn, int *outConnected) { (void)conn; if (outConnected) *outConnected = 0; return GP_NO_ERROR; }
GPResult gpDeleteProfile(GPConnection *conn, int flags) { (void)conn; (void)flags; return GP_NO_ERROR; }
GPResult gpDeleteBuddy(GPConnection *conn, GPProfile profile) { (void)conn; (void)profile; return GP_NO_ERROR; }
GPResult gpAuthBuddyRequest(GPConnection *conn, GPProfile profile) { (void)conn; (void)profile; return GP_NO_ERROR; }
GPResult gpDenyBuddyRequest(GPConnection *conn, GPProfile profile) { (void)conn; (void)profile; return GP_NO_ERROR; }
char *gpGetValueString(GPStatus *status, const char *property) { (void)status; (void)property; return (char*)""; }
int gpValueExists(GPStatus *status, const char *property) { (void)status; (void)property; return 0; }
