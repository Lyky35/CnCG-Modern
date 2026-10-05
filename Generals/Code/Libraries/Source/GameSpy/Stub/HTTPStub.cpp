#include "GameSpy/ghttp/ghttp.h"

void ghttpStartup(void) {}
void ghttpCleanup(void) {}
GHTTPRequest ghttpGet(const char *url, GHTTPBool blocking, GHTTPRequestCallback callback, void *param) { return NULL; }
GHTTPRequest ghttpHead(const char *url, GHTTPBool blocking, GHTTPRequestCallback callback, void *param) { return NULL; }
void ghttpThink(void) {}
void ghttpSetProxy(const char *proxy) {}
const char *ghttpGetHeaders(GHTTPRequest request) { return NULL; }
