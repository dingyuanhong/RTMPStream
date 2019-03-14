#include <stdlib.h>
#include <WinSock2.h>
#include <assert.h>

#include "../capture/screen_capturer.h"
#include "../capture/desktop_capture_options.h"
#include "../capture/desktop_geometry.h"
#include "../capture/desktop_region.h"
#include "../capture/desktop_frame.h"

#include "../Stream/x264_encoder.h"
#include "../Stream/librtmp_send264.h"

#include "../Stream/x264Encoder.h"
#include "../Stream/rtmp_push.h"
#include "../Stream/rtmp_pull.h"

extern "C" {
#include "../librtmp/log.h"
#include "../librtmp/rtmp.h"
}

#include "libyuv.h"

#pragma comment(lib,"ws2_32.lib")

//#pragma comment(lib,"zlibwapi.lib")
#pragma comment(lib,"zlibstat.lib")
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libyuv.lib")
#pragma comment(lib,"x264.dll.lib")

#pragma comment(lib,"capture.lib")
#pragma comment(lib,"librtmp.lib")
#pragma comment(lib,"Stream.lib")



void initSock() {
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
}
void cleanSock() {
	WSACleanup();
}

void pushDirect(char * url);
void pushAsync(char * url);
void pushNew(char * url);

void pullRaw(char * url, char * output);
void pullH264(char * url);

int main(int argc,char *argv[]) {
	if (argc <= 1) {
		return -1;
	}

	initSock();

	if (strcmp(argv[1], "pushd") == 0) {
		if (argc < 2) return -1;
		pushDirect(argv[2]);
	}
	else if (strcmp(argv[1], "pusha") == 0) {
		if (argc < 2) return -1;
		pushAsync(argv[2]);
	}
	else if (strcmp(argv[1], "push") == 0) {
		if (argc < 2) return -1;
		pushNew(argv[2]);
	}
	else if (strcmp(argv[1], "pullH264") == 0) {
		if (argc < 2) return -1;
		pullH264(argv[2]);
	}
	else if (strcmp(argv[1], "pullRaw") == 0) {
		if (argc < 3) return -1;
		pullRaw(argv[2], argv[3]);
	}

	cleanSock();
	return 0;
}