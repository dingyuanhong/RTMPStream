#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <WinSock2.h>
#include <assert.h>

//单位微秒
static void usleep(uint32_t usec)
{
	struct timeval timeout;
	fd_set fds;
	FD_ZERO(&fds);
	int32_t fd = socket(AF_INET, SOCK_DGRAM, 0);
	FD_SET(fd, &fds);
	timeout.tv_sec = usec / 1000000;
	timeout.tv_usec = usec % 1000000;
	int ret = select(0, NULL, NULL, &fds, &timeout);
	if (0 > ret)
	{
	}
	closesocket(fd);
}

static int readAble(int fd, uint32_t usec)
{
	struct timeval timeout;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	timeout.tv_sec = usec / 1000000;
	timeout.tv_usec = usec % 1000000;
	int ret = select(0, &fds, NULL, &fds, &timeout);
	return ret;
}

#include <time.h>      //添加头文件
static int64_t getCurrentTime()
{
	struct timeval tv;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tv.tv_sec = clock;
	tv.tv_usec = wtm.wMilliseconds * 1000;
	return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
}