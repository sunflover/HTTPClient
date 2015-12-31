#include "HttpClient.h"
#include <iostream>

void main()
{
#if defined(_WIN32)
	WORD    version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0)
		return;//failed init network
#endif // _WIN32


	HTTPClient http;
	HTTPRequest req;
	HTTPResponse rep;
	std::string strData;
	req.url = "http://www.qq.com/";
	req.setHead(HTTP_HEAD_REFERER, "http://www.qq.com/");
	if (http.get(req, rep, strData))
	{
		std::cout << rep.code << std::endl;
	}

#if defined(_WIN32)
	WSACleanup();
#endif // _WIN32

	return;
}