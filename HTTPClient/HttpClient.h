#pragma once
#include "HTTPSession.h"


enum HTTPRESPONSE_CODE
{
	HTTPRESPONSE_000 = 0,
	HTTPRESPONSE_200 = 200,
	HTTPRESPONSE_204 = 204,
	HTTPRESPONSE_206 = 206,
	HTTPRESPONSE_301 = 301,
	HTTPRESPONSE_302 = 302,
	HTTPRESPONSE_400 = 400,
	HTTPRESPONSE_403 = 403,
	HTTPRESPONSE_404 = 404,
	HTTPRESPONSE_413 = 413,
	HTTPRESPONSE_500 = 500,
};

enum HTTP_REQ_TYPE {
	HTTP_DEFAULT,
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE 
};

enum HTTP_STATUS
{
	HTTP_STATUS_DEFAULT,
	HTTP_STATUS_CONNECT_FAILED,
	HTTP_STATUS_CONNECT_SUCCESS,
	HTTP_STATUS_CONNECT_STOP,
	HTTP_STATUS_CONNECT_CLOSE,
	HTTP_STATUS_IO_ERROR,
	HTTP_STATUS_PARSE_URL_FAILED
};

//////////////////////////////////////////////////////////////////////////
#define HTTP_REQUEST_GET	"GET"
#define HTTP_REQUEST_POST	"POST" 
#define HTTP_REQUEST_PUT	"PUT"
#define HTTP_REQUEST_DELETE	"DELETE"
//////////////////////////////////////////////////////////////////////////
#define HTTP_VERSION_1_1	"HTTP/1.1"
//////////////////////////////////////////////////////////////////////////
#define HTTP_HEAD_HOST	"Host"
#define HTTP_HEAD_ACCEPT	"Accept"
#define HTTP_HEAD_REFERER	"Referer"
#define HTTP_HEAD_ACCEPT_LANGUAGE	"Accept-Language"
#define HTTP_HEAD_ACCEPT_ENCODING	"Accept-Encoding"
#define HTTP_HEAD_ACCEPT_CHARSET	"Accept-Charset"
#define HTTP_HEAD_ACCEPT_RANGES	"Accept-Ranges"
#define HTTP_HEAD_USER_AGENT	"User-Agent"
#define HTTP_HEAD_COOKIE	"Cookie"
#define HTTP_HEAD_CONTENT_TYPE	"Content-Type"
#define HTTP_HEAD_DATE	"Date"
#define HTTP_HEAD_CHARSET	"utf-8"
#define HTTP_HEAD_CONTENT_LENGTH	"Content-Length"
#define HTTP_HEAD_DATE	"Date"
#define HTTP_HEAD_KEEP_ALIVE	"Keep-Alive"
#define HTTP_HEAD_CONNECTION	"Connection"
#define HTTP_HEAD_SERVER	"Server"
#define HTTP_HEAD_LOCATION	"Location"
#define HTTP_HEAD_SET_COOKIE	"Set-Cookie"
#define HTTP_HEAD_CONTENT_ENCODING	"Content-Encoding"
#define HTTP_HEAD_TRANSFER_ENCODING	"Transfer-Encoding"

#define HTTP_HEAD_CONTENT_DISPOSITION	"Content-Disposition"
#define HTTP_HEAD_CONTENT_TRANSFER_ENCODING	"Content-Transfer-Encoding"

struct MultiPart
{
	std::map<std::string, std::string> m_headMap;
	std::string data;

	void setHead(const std::string& key, const std::string& value)
	{
		m_headMap[key] = value;
	}

	std::string makeHeader()
	{
		std::ostringstream oss;
		for (std::map<std::string, std::string>::iterator st = m_headMap.begin(); st != m_headMap.end(); ++ st)
		{
			oss << st->first << ": " << st->second << "\r\n";
		}
		oss << "\r\n";
		return oss.str();
	}
};

struct HTTPMultipart
{
	std::string strBoundary;
	std::map<uint32_t, MultiPart> m_multiPartMap;

	void makeMultiPart(std::string& strBody)
	{
		std::ostringstream oss;
		for (std::map<uint32_t, MultiPart>::iterator it = m_multiPartMap.begin(); it != m_multiPartMap.end(); ++ it)
		{
			if (it == m_multiPartMap.begin())
			{
				oss << "--" << strBoundary << "\r\n";//first boundary
			}
			else
			{
				oss << "\r\n--" << strBoundary << "\r\n";
			}
			MultiPart& part = it->second;
			oss << part.makeHeader();
			oss << part.data;
		}
		oss << "\r\n--" << strBoundary << "--" << "\r\n";//last boundary
		strBody.append(oss.str());
	}
};

struct HTTPRequest
{
	std::string version;
	std::string url;
	std::string host;
	uint16_t port;
	std::string strRelative;
	std::map<std::string, std::string> m_headMap;

	HTTPRequest()
	: version(HTTP_VERSION_1_1)
	, port(80)
	, strRelative("/")
	{
		m_headMap[HTTP_HEAD_ACCEPT] = "*/*";
		m_headMap[HTTP_HEAD_ACCEPT_LANGUAGE] = "zh-CN";
		m_headMap[HTTP_HEAD_USER_AGENT] = "SDK C++ http client";
		m_headMap[HTTP_HEAD_CONNECTION] = "Keep-Alive";
	}

	void setHead(const std::string& key, const std::string& value)
	{
		m_headMap[key] = value;
	}
};

struct HTTPResponse
{
	HTTPRESPONSE_CODE code;
	bool bChunked;
	bool bChunkedEnd;
	uint32_t uContentLength;
	uint32_t uReadLength;
	uint32_t uChunkLength;
	std::map<std::string, std::string> m_headMap;

	HTTPResponse()
	{
		bChunked = false;
		bChunkedEnd = false;
		uContentLength = 0;
		uReadLength = 0;
		uChunkLength = 0;
		code = HTTPRESPONSE_000;
	}

	void setHead(const std::string& key, std::string& value)
	{
		m_headMap[key] = value;
	}
};

class HTTPClient
{
public:
	HTTPClient(void);
	~HTTPClient(void);

public:
	bool get(HTTPRequest& req, HTTPResponse& rep, std::string& strData);
	bool post(HTTPRequest& req, HTTPResponse& rep, const std::string& strData, std::string& strResult);
	bool startPost(HTTPSession& session, HTTPRequest& req);
	bool postData(HTTPSession& session, const std::string& data);
	void stopPost(HTTPSession& session, HTTPResponse& rep, std::string& strResult);
	bool startGet(HTTPSession& session, HTTPResponse& rep, HTTPRequest& req);
	bool getData(HTTPSession& session,  HTTPResponse& rep, std::string& data);
	void stopGet(HTTPSession& session);

public:
	void makeMultipartBody(HTTPMultipart& multipart, std::string& strBody);

private:
	bool parseUrl(HTTPRequest& req);
	void makeHeader(HTTP_REQ_TYPE& type, HTTPRequest& req, std::string &strHead);
	bool sendHeader(HTTP_REQ_TYPE& type, HTTPSession& session, HTTPRequest& req);
	bool readHeader(HTTPSession& session, HTTPResponse& rep);
	bool sendBody(HTTPSession& session, const std::string& strData);
	bool readBody(HTTPSession& session, HTTPResponse& rep, std::string& strData);
	void parseLine(const std::string& line, std::string& key, std::string& value);
	uint32_t hostToLong(std::string host);

};
