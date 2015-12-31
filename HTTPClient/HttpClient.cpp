#include "HttpClient.h"
//************************************
// 文件名称：HttpClient.cpp
// 功能说明：
// QQ    群：181926437
// QQ    号：515199494
// 作    者：流年 
// 日    期：2015/08/31
//************************************

HTTPClient::HTTPClient(void)
{
}


HTTPClient::~HTTPClient(void)
{
}

bool HTTPClient::get(HTTPRequest& req, HTTPResponse& rep, std::string& strData)
{
	do 
	{
		if (!parseUrl(req))
		{
			break;
		}
		HTTP_REQ_TYPE type = HTTP_GET;
		HTTPSession session;
		if (!sendHeader(type, session, req))
		{
			break;
		}
		if (!readHeader(session, rep))
		{
			break;
		}
		if (!readBody(session, rep, strData))
		{
			break;
		}
		return true;
	} while (false);
	return false;
}

bool HTTPClient::post(HTTPRequest& req, HTTPResponse& rep, const std::string& strData, std::string& strResult)
{
	do 
	{
		if (!parseUrl(req))
		{
			break;
		}
		HTTP_REQ_TYPE type = HTTP_POST;
		HTTPSession session;
		if (!sendHeader(type, session, req))
		{
			break;
		}
		if (!sendBody(session, strData))
		{
			break;
		}
		if (!readHeader(session, rep))
		{
			break;
		}
		if (rep.uContentLength > 0)
		{
			readBody(session, rep, strResult);
		}
		return true;
	} while (false);
	return false;
}

bool HTTPClient::startPost(HTTPSession& session, HTTPRequest& req)
{
	do 
	{
		if (!parseUrl(req))
		{
			break;
		}
		HTTP_REQ_TYPE type = HTTP_POST;
		if (!sendHeader(type, session, req))
		{
			break;
		}
		return true;
	} while (false);
	return false;
}

bool HTTPClient::postData(HTTPSession& session, const std::string& data)
{
	return sendBody(session, data);
}

void HTTPClient::stopPost(HTTPSession& session, HTTPResponse& rep, std::string& strResult)
{
	readHeader(session, rep);
	if (rep.uContentLength > 0)
	{
		readBody(session, rep, strResult);
	}
	session.closeSession();
}

bool HTTPClient::startGet(HTTPSession& session, HTTPResponse& rep, HTTPRequest& req)
{
	do 
	{
		if (!parseUrl(req))
		{
			break;
		}
		HTTP_REQ_TYPE type = HTTP_GET;
		if (!sendHeader(type, session, req))
		{
			break;
		}
		if (!readHeader(session, rep))
		{
			break;
		}
		return true;
	} while (false);
	return false;
}

bool HTTPClient::getData(HTTPSession& session, HTTPResponse& rep, std::string& data)
{
	return readBody(session, rep, data);
}

void HTTPClient::stopGet(HTTPSession& session)
{
	session.closeSession();
}

bool HTTPClient::parseUrl(HTTPRequest& req)
{
	do 
	{
		if (req.url.length() <= strlen("http://"))
		{
			break;
		}
		char* url = (char*)req.url.c_str();
		char* result = strstr(url, "http://");
		if (!result || result != url)
		{
			break;
		}
		url += strlen("http://");
		result = strstr(url, "/");
		char* colon = strstr(url, ":");
		if (colon && result && colon > result)
		{
			break;
		}
		if (result && colon)
		{
			req.host = std::string(url, colon - url);
			std::string port(colon + 1, result - colon - 1);
			req.port = (uint16_t)atoi(port.c_str());
			req.strRelative = std::string(result);
		}
		else if (result && !colon)
		{
			req.host = std::string(url, result - url);
			req.strRelative = std::string(result);
		}
		else if (!result && colon)
		{
			req.host = std::string(url, colon - url);
			std::string port(colon + 1);
			req.port = (uint16_t)atoi(port.c_str());
		}
		else if (!result && !colon)
		{
			req.host = std::string(url);
		}
		return true;
	} while (false);
	return false;
}

void HTTPClient::makeHeader(HTTP_REQ_TYPE& type, HTTPRequest& req, std::string &strHead)
{
	std::string strHttpType;
	switch (type)
	{
	case HTTP_GET: strHttpType = HTTP_REQUEST_GET; break;
	case HTTP_POST: strHttpType = HTTP_REQUEST_POST; break;
	case HTTP_PUT: strHttpType = HTTP_REQUEST_PUT; break;
	case HTTP_DELETE: strHttpType = HTTP_REQUEST_DELETE; break;
	}
	std::ostringstream oss;
	oss << strHttpType << " " << req.strRelative << " " << req.version << "\r\n";
	oss << HTTP_HEAD_HOST << ": " << req.host;
	if (req.port != 80)
	{
		oss << ":" << req.port;
	}
	oss << "\r\n";
	for (std::map<std::string, std::string>::iterator it = req.m_headMap.begin(); it != req.m_headMap.end(); ++ it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	strHead.append(oss.str());
}

void HTTPClient::makeMultipartBody(HTTPMultipart& multipart, std::string& strBody)
{
	std::ostringstream oss;
	for (std::map<uint32_t, MultiPart>::iterator it = multipart.m_multiPartMap.begin(); it != multipart.m_multiPartMap.end(); ++ it)
	{
		if (it == multipart.m_multiPartMap.begin())
		{
			oss << "--" << multipart.strBoundary << "\r\n";//first boundary
		}
		else
		{
			oss << "\r\n--" << multipart.strBoundary << "\r\n";
		}
		MultiPart& part = it->second;
		for (std::map<std::string, std::string>::iterator st = part.m_headMap.begin(); st != part.m_headMap.end(); ++ st)
		{
			oss << st->first << ": " << st->second << "\r\n";
		}
		oss << "\r\n";
		oss << part.data;
	}
	oss << "\r\n--" << multipart.strBoundary << "--" << "\r\n";//last boundary
	strBody.append(oss.str());
}

bool HTTPClient::sendHeader(HTTP_REQ_TYPE& type, HTTPSession& session, HTTPRequest& req)
{
	do 
	{
		std::string strHttpHeader;
		makeHeader(type, req, strHttpHeader);
		uint32_t ip = hostToLong(req.host);
		uint16_t port = htons(req.port);
		if (!session.connect(ip, port))
		{
			break;
		}

		bool bRet = true;
		std::string data = strHttpHeader;
		while (data.length() > 0)
		{
			int nSend = session.send(data.c_str(),data.length());
			if (nSend < 0)
			{
				bRet = false;
				break;
			}
			else if (nSend > 0)
			{
				data = data.substr(nSend,data.length() - nSend);
			}
		}
		return bRet;
	} while (false);
	return false;
}

bool HTTPClient::readHeader(HTTPSession& session, HTTPResponse& rep)
{
	do 
	{
		for (;;)
		{
			std::string line;
			int ret = session.readLine(line, 102400);
			if (ret < 0)
			{
				break;
			}
			else if (ret == 0)
			{
				break;
			}
			if (line.size() == strlen("\r\n") && line.data() == strstr(line.data(), "\r\n"))
			{
				break;
			}
			else
			{
				std::string strKey, strValue;
				parseLine(line, strKey, strValue);
				if (strKey.data() == strstr(strKey.data(), "HTTP"))
				{
					size_t pos = strValue.find(" ");
					if (pos != strValue.npos)
					{
						std::string strCode = strValue.substr(0,pos); 
						rep.code = (HTTPRESPONSE_CODE)atoi(strCode.c_str());
					}
				}
				else 
				{
					rep.setHead(strKey, strValue);
					if (strKey == HTTP_HEAD_TRANSFER_ENCODING)
					{
						if (strValue == "chunked")
						{
							rep.bChunked = true;
						}
					}
					else if (strKey == HTTP_HEAD_CONTENT_LENGTH)
					{
						rep.uContentLength = (uint32_t)atoi(strValue.c_str());
					}
				}
			}
		}
		return true;
	} while (false);
	return false;
}

bool HTTPClient::sendBody(HTTPSession& session, const std::string& strData)
{
	do 
	{
		bool bRet = true;
		std::string data = strData;
		while (data.length() > 0)
		{
			int nSend = session.send(data.c_str(),data.length());
			if (nSend < 0)
			{
				bRet = false;
				break;
			}
			else if (nSend > 0)
			{
				data = data.substr(nSend,data.length() - nSend);
			}
		}
		return bRet;
	} while (false);
	return false;
}

bool HTTPClient::readBody(HTTPSession& session, HTTPResponse& rep, std::string& strData)
{
	do 
	{
		int ret = 0;
		do 
		{
			if (rep.bChunked)
			{
				if (rep.uChunkLength == 0)
				{
					std::string line;
					ret = session.readLine(line, 1024);
					if (ret > 0)
					{
						if (rep.bChunkedEnd)
						{
							break;
						}
						if (line == "\r\n")
						{
							continue;
						}
						int size = 0;
						sscanf(line.c_str(), "%x", &size);
						if (size >= 0)
						{
							rep.uChunkLength = size;
							rep.bChunkedEnd = size == 0 ? true : false;
							continue;
						}
					}
				}
				else
				{
					std::string chunk;
					ret = session.readLimit(chunk, rep.uChunkLength);
					if (ret > 0)
					{
						rep.uReadLength += ret;
						rep.uChunkLength -= ret;
						strData.append(chunk);
					}
				}
			}
			else
			{
				char data[10240 + 1] = {0};
				ret = session.read(data, 10240);
				if (ret > 0)
				{
					rep.uReadLength += ret;
					strData.append(data, ret);
					if (rep.uReadLength >= rep.uContentLength)
					{
						break;
					}
				}
			}
		} while (ret >= 0);

		return ret > 0 ? true : false;
	} while (false);
	return false;
}

void HTTPClient::parseLine(const std::string& line, std::string& key, std::string& value)
{
	std::string delimiters = "\r\n";
	size_t pos1 = line.find_first_not_of(delimiters);
	size_t pos2 = line.find_last_not_of(delimiters);
	std::string strLine = line;
	if(std::string::npos != pos1 && std::string::npos != pos2)
	{
		strLine = line.substr(pos1,pos2 - pos1 + 1);
	}
	char* start = (char*)strLine.data();
	if (strstr(strLine.data(), "HTTP") == strLine.data())
	{
		char* pos = strstr(start, " ");
		if (pos)
		{
			key.append(start, pos - start);
			pos += strlen(" ");
			value.append(pos);
		}
	}
	else
	{
		char* pos = strstr(start, ": ");
		if (pos)
		{
			key.append(start, pos - start);
			pos += strlen(": ");
			value.append(pos);
		}
	}
}

uint32_t HTTPClient::hostToLong(std::string host)
{
	bool bDomain = false;
	int idx = 0;
	int len = host.length();
	while(idx < len)
	{
		if ((host[idx] >= 'a' || host[idx] >= 'A') && (host[idx] <= 'z' || host[idx] <= 'Z'))
		{
			bDomain = true;
			break;
		}
		idx ++;
	}
	if (bDomain)
	{
		struct hostent *he;
		if ((he = gethostbyname(host.c_str())))
		{
			struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;
			in_addr &ip_addr = *addr_list[0];
			return ip_addr.s_addr;
		}
		return 0;
	}
	return inet_addr(host.c_str());
}