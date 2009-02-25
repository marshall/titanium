/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _HTTP_CLIENT_BINDING_H_
#define _HTTP_CLIENT_BINDING_H_

#include <kroll/kroll.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Thread.h>

namespace ti
{
	class HTTPClientBinding : public StaticBoundObject
	{
	public:
		HTTPClientBinding(Host* host, std::string &url, SharedBoundMethod callback);
		virtual ~HTTPClientBinding();
	private:
		Host* host;
		SharedBoundObject global;
		SharedBoundMethod callback;
		Poco::Thread *thread;
		std::string url;
		Poco::Net::HTTPResponse* response;
		
		static void Run(void*);
		void Cancel(const ValueList& args, SharedValue result);
		void GetHeader(const ValueList& args, SharedValue result);
	};
}

#endif
