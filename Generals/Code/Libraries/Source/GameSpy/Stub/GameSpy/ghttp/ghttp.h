#pragma once

#include <windows.h>

typedef void* GHTTPRequest;
typedef int GHTTPBool;
typedef int GHTTPState;

#define GHTTPTrue 1
#define GHTTPFalse 0

enum {
    GHTTPError = 0,
    GHTTPInProgress = 1,
    GHTTPFileComplete = 2,
    GHTTPFileFailed = 3,
    GHTTPFileWrite = 4
};

typedef void (*GHTTPRequestCallback)(GHTTPRequest request, GHTTPState state, char *buffer, int bufferLen, void *param);

void ghttpStartup(void);
void ghttpCleanup(void);
GHTTPRequest ghttpGet(const char *url, GHTTPBool blocking, GHTTPRequestCallback callback, void *param);
GHTTPRequest ghttpHead(const char *url, GHTTPBool blocking, GHTTPRequestCallback callback, void *param);
void ghttpThink(void);
void ghttpSetProxy(const char *proxy);
const char *ghttpGetHeaders(GHTTPRequest request);
