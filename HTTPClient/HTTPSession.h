#pragma once
#include <string>
#include <sstream>
#include <map>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif
#else // unix or linux
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>
#include<netdb.h>

#ifndef SOCKET
#define SOCKET  int
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#endif

class HTTPSession
{
public:
	HTTPSession(void);
	~HTTPSession(void);

public:
	bool connect(uint32_t ip, uint16_t port);
	void closeSession();
	int send(const char* data, const int len);
	int read(char* data, const int len);
	int readLine(std::string& line, const int len = 0);
	int readLimit(std::string& limit, const int len);

private:
	bool initSocket();

private:
	SOCKET m_sock;
};
