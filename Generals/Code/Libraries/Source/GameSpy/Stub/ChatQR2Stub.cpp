/*
**	No-op implementations for the GameSpy Chat / QR2 / GP-persist helper surface
**	declared in the stub SDK headers. Multiplayer services are unavailable in
**	this build; these definitions exist so the engine links and the calls degrade.
*/

#include "GameSpy/Peer/Peer.h"
#include "GameSpy/gstats/gpersist.h"
#include <string.h>

/* ---- QR2 ---- */
void qr2_buffer_add(qr2_buffer_t buffer, const char *keyValuePair) { (void)buffer; (void)keyValuePair; }
void qr2_buffer_add_int(qr2_buffer_t buffer, int value) { (void)buffer; (void)value; }
void qr2_keybuffer_add(qr2_keybuffer_t keyBuffer, int key) { (void)keyBuffer; (void)key; }
void qr2_register_key(int key, const char *name) { (void)key; (void)name; }

/* ---- Chat ---- */
static char g_challenge[] = "";
static char g_auth[] = "";

char *GetChallenge(void *session) { (void)session; return g_challenge; }
int GenerateAuth(const char *challenge, char *password, char *outAuth)
{ (void)challenge; (void)password; if (outAuth) outAuth[0] = 0; return 0; }
int NewGame(unsigned int maxPlayers) { (void)maxPlayers; return 0; }
int SendGameSnapShot(void *handle, const char *snapshot, int finalFlag) { (void)handle; (void)snapshot; (void)finalFlag; return 0; }
int FreeGame(void *handle) { (void)handle; return 0; }
void chatSetLocalIP(unsigned int localIP) { (void)localIP; }

/* ---- GP persist ---- */
int SetPersistDataValues(int localid, int profileid, persisttype_t type, int index, const char *keysValues,
                         PersistSetCallback callback, void *param)
{ (void)localid; (void)profileid; (void)type; (void)index; (void)keysValues; (void)callback; (void)param; return 0; }
int PreAuthenticatePlayerPM(int localid, int profileid, char *authToken, GPSPreAuthCallback callback, void *param)
{ (void)localid; (void)profileid; (void)authToken; (void)callback; (void)param; return 0; }
int PreAuthenticatePlayerCD(int localid, const char *cdkey, char *cdkeyHash, char *authToken, GPSPreAuthCallback callback, void *param)
{ (void)localid; (void)cdkey; (void)cdkeyHash; (void)authToken; (void)callback; (void)param; return 0; }

/* ---- gstats SBServer accessors ---- */
static const char *g_empty = "";

const char *SBServerGetStringValue(SBServer server, const char *key, const char *defaultValue)
{ (void)server; (void)key; return defaultValue ? defaultValue : g_empty; }
int SBServerGetIntValue(SBServer server, const char *key, int defaultValue)
{ (void)server; (void)key; return defaultValue; }
const char *SBServerGetPlayerStringValue(SBServer server, int index, const char *key, const char *defaultValue)
{ (void)server; (void)index; (void)key; return defaultValue ? defaultValue : g_empty; }
int SBServerGetPlayerIntValue(SBServer server, int index, const char *key, int defaultValue)
{ (void)server; (void)index; (void)key; return defaultValue; }
int SBServerHasBasicKeys(SBServer server) { (void)server; return 0; }
int SBServerHasFullKeys(SBServer server) { (void)server; return 0; }
unsigned int SBServerGetPublicInetAddress(SBServer server) { (void)server; return 0; }
unsigned int SBServerGetPrivateInetAddress(SBServer server) { (void)server; return 0; }
unsigned short SBServerGetPrivateQueryPort(SBServer server) { (void)server; return 0; }
int SBServerEnumKeys(SBServer server, void (*enumFunc)(char *key, char *val, void *param), void *param)
{ (void)server; (void)enumFunc; (void)param; return 0; }
SBServer SBServers(PEER peer) { (void)peer; return NULL; }
SBServer SBServersNext(SBServer current, SBServer *nextOut) { (void)current; if (nextOut) *nextOut = NULL; return NULL; }
const char *SBServerName(SBServer server) { (void)server; return g_empty; }

/* GameSpy global game-descriptor strings (referenced by persistent-storage code) */
char gcd_gamename[128] = "ccgenerals";
char gcd_secret_key[128] = "";

extern "C" {
// QR2 hosting status query used by the in-game chat HUD
int getQR2HostingStatus(void) { return 0; }
}
