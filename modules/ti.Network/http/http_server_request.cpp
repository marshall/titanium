/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "http_server_request.h"
#include "http_server_response.h"
#include <Poco/Buffer.h>

namespace ti
{
	HttpServerRequest::HttpServerRequest(Host *host, SharedKMethod callback, Poco::Net::HTTPServerRequest &request) :
		host(host),callback(callback),request(request)
	{
		SetMethod("getMethod",&HttpServerRequest::GetMethod);
		SetMethod("getVersion",&HttpServerRequest::GetVersion);
		SetMethod("getURI",&HttpServerRequest::GetURI);
		SetMethod("getContentType",&HttpServerRequest::GetContentType);
		SetMethod("getContentLength",&HttpServerRequest::GetContentLength);
		SetMethod("getHeader",&HttpServerRequest::GetHeader);
		SetMethod("hasHeader",&HttpServerRequest::HasHeader);
		SetMethod("read",&HttpServerRequest::Read);
	}
	HttpServerRequest::~HttpServerRequest()
	{
	}
	void HttpServerRequest::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
	{
		SharedKObject req = SharedKObject(this);
		SharedKObject resp = new HttpServerResponse(response);
		ValueList args;
		args.push_back(Value::NewObject(req));
		args.push_back(Value::NewObject(resp));
		host->InvokeMethodOnMainThread(callback,args);
	}
	void HttpServerRequest::GetMethod(const ValueList& args, SharedValue result)
	{
		std::string method = request.getMethod();
		result->SetString(method);
	}
	void HttpServerRequest::GetVersion(const ValueList& args, SharedValue result)
	{
		std::string version = request.getVersion();
		result->SetString(version);
	}
	void HttpServerRequest::GetURI(const ValueList& args, SharedValue result)
	{
		std::string uri = request.getURI();
		result->SetString(uri);
	}
	void HttpServerRequest::GetContentType(const ValueList& args, SharedValue result)
	{
		std::string ct = request.getContentType();
		result->SetString(ct);
	}
	void HttpServerRequest::GetContentLength(const ValueList& args, SharedValue result)
	{
		result->SetInt(request.getContentLength());
	}
	void HttpServerRequest::GetHeader(const ValueList& args, SharedValue result)
	{
		std::string name = args.at(0)->ToString();
		if (request.has(name))
		{
			std::string value = request.get(name);
			result->SetString(value);
		}
		else
		{
			result->SetNull();
		}
	}
	void HttpServerRequest::HasHeader(const ValueList& args, SharedValue result)
	{
		std::string name = args.at(0)->ToString();
		result->SetBool(request.has(name));
	}
	void HttpServerRequest::Read(const ValueList& args, SharedValue result)
	{
		std::istream &in = request.stream();
		if (in.eof() || in.fail())
		{
			result->SetNull();
			return;
		}
		int max_size = 8096;
		if (args.size()==1)
		{
			max_size = args.at(0)->ToInt();
		}
		char *buf = new char[max_size];
		in.read(buf,max_size);
		std::streamsize count = in.gcount();
		if (count == 0)
		{
			result->SetNull();
		}
		else
		{
			result->SetObject(new Blob(buf,count));
		}
		delete [] buf;
	}

}