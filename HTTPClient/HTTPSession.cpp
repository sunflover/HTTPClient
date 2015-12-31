#include "HTTPSession.h"
//************************************
// 文件名称：HttpClient.cpp
// 功能说明：
// QQ    群：181926437
// QQ    号：515199494
// 作    者：流年 
// 日    期：2015/08/31
//************************************

HTTPSession::HTTPSession(void)
: m_sock(INVALID_SOCKET)
{

}

HTTPSession::~HTTPSession(void)
{
	closeSession();
}

bool HTTPSession::connect(uint32_t ip, uint16_t port)
{
	do 
	{
		if (!initSocket())
		{
			break;
		}
		struct sockaddr_in dstAddr;
		dstAddr.sin_family = AF_INET;
		dstAddr.sin_addr.s_addr = ip;
		dstAddr.sin_port = port;
		if (::connect(m_sock, (struct sockaddr*)&dstAddr, sizeof(dstAddr)) == -1)
		{
			break;
		}
		return true;
	} while (false);
	return false;
}

void HTTPSession::closeSession()
{
	if (m_sock != INVALID_SOCKET)
	{
#if defined(_WIN32)
		closesocket(m_sock);
#else
		close(m_sock);
#endif
	}
	m_sock = INVALID_SOCKET;
}

int HTTPSession::send(const char* data, const int len)
{
	return ::send(m_sock, data, len, 0);
}

int HTTPSession::read(char* data, const int len)
{
	return ::recv(m_sock, data, len, 0);
}

int HTTPSession::readLine(std::string& line, const int len)
{
	line.clear();
	char ch = 0;
	uint32_t nSize = 0;
	do 
	{
		if (len > 0 && nSize >= len)
		{
			break;
		}
		int ret = read(&ch, 1);
		if (ret == 1)
		{
			nSize ++;
			line.append(1, ch);
		}
		else if(ret == 0)
		{
			break;
		}
		else
		{
			nSize = (nSize == 0 ? -1 : nSize);
			break;
		}
	} while (ch != '\n');
	return nSize;
}

int HTTPSession::readLimit(std::string& limit, const int len)
{
	limit.clear();
	//len 1048
	int size = len;
	int nRead = size > 1024 ? 1024 : size;
	char data[1024] = {0};
	int ret = 0;
	while(size > 0 && (ret = read(data, nRead)) > 0)
	{
		size -= ret;
		nRead = size > 1024 ? 1024 : size;
		limit.append(data, ret);
	}
	return len - size;
}

bool HTTPSession::initSocket()
{
	if (m_sock == INVALID_SOCKET)
	{
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
	}
	return m_sock != INVALID_SOCKET;
}