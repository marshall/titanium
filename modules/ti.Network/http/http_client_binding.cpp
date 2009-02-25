/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "http_client_binding.h"
#include <cstring>
#include <iostream>
#include "../network_binding.h"

#ifdef OS_OSX
#import <Cocoa/Cocoa.h>
#endif

namespace ti
{
	HTTPClientBinding::HTTPClientBinding(Host* host, std::string &url, SharedBoundMethod callback) : 
		host(host),global(host->GetGlobalObject()),callback(callback),thread(NULL),url(url),response(NULL)
	{
		this->Set("connected",Value::NewBool(false));
		this->SetMethod("cancel",&HTTPClientBinding::Cancel);
		this->SetMethod("getHeader",&HTTPClientBinding::GetHeader);
		this->thread = new Poco::Thread();
		this->thread->start(&HTTPClientBinding::Run,(void*)this);
	}
	HTTPClientBinding::~HTTPClientBinding()
	{
		if (this->thread!=NULL)
		{
			delete this->thread;
			this->thread = NULL;
		}
	}
	void HTTPClientBinding::Run (void* p)
	{
#ifdef OS_OSX
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif
		HTTPClientBinding *binding = reinterpret_cast<HTTPClientBinding*>(p);

#ifdef DEBUG
		std::cout << "HTTPClientBinding:: starting => " << binding->url << std::endl;
#endif

		int max_redirects = 5;
		std::string url = binding->url;
		for (int x=0;x<max_redirects;x++)
		{
			Poco::URI uri(url);
			std::string path(uri.getPathAndQuery());
			if (path.empty()) path = "/";
			binding->Set("connected",Value::NewBool(true));
			Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
			Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
			const char* ua = binding->global->Get("userAgent")->IsString() ? binding->global->Get("userAgent")->ToString() : NULL;
#ifdef DEBUG
			std::cout << "HTTPClientBinding:: userAgent = " << ua << std::endl;
#endif
			if (ua)
			{
				req.set("User-Agent",ua);
			}
			else
			{
				// crap, this means we don't have one for some reason -- just fake it
				req.set("User-Agent","Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_6; en-us) AppleWebKit/528.7+ (KHTML, like Gecko) "PRODUCT_NAME"/"STRING(PRODUCT_VERSION));
			}
			session.sendRequest(req);
			Poco::Net::HTTPResponse res;
			std::istream& rs = session.receiveResponse(res);
			int total = res.getContentLength();
#ifdef DEBUG
			std::cout << "HTTPClientBinding:: response received = " << total << std::endl;
			std::cout << res.getStatus() << " " << res.getReason() << std::endl;
#endif
			binding->Set("status",Value::NewInt(res.getStatus()));
			binding->Set("message",Value::NewString(res.getReason().c_str()));

			if (res.getStatus() == 301 || res.getStatus() == 302)
			{
				if (!res.has("Location"))
				{
					break;
				}
				url = res.get("Location");
#ifdef DEBUG
				std::cout << "redirect to " << url << std::endl;
#endif				
				continue;
			}
			SharedValue totalValue = Value::NewInt(total);
			SharedValue self = Value::NewObject(binding);
			binding->response = &res;
			int count = 0;
			char buf[8096];
			while(!rs.eof() && binding->Get("connected")->ToBool())
			{
				try
				{
					rs.read((char*)&buf,8095);
					std::streamsize c = rs.gcount();
					if (c > 0)
					{
						buf[c]='\0';
						count+=c;

						ValueList args;
						args.push_back(self); // reference to us
						args.push_back(Value::NewInt(count)); // total count
						args.push_back(totalValue); // total size
						args.push_back(Value::NewObject(new Blob(buf,c))); // buffer
						args.push_back(Value::NewInt(c)); // buffer length

						binding->host->InvokeMethodOnMainThread(binding->callback,args);
					}
				}
				catch(std::exception &e)
				{
					std::cerr << "Caught exception dispatching HTTP callback, Error: " << e.what() << std::endl;
				}
				catch(...)
				{
					std::cerr << "Caught unknown exception dispatching HTTP callback" << std::endl;
				}
				if (rs.eof()) break;
			}
			break;
		}
		binding->response = NULL;
		binding->Set("connected",Value::NewBool(false));
		NetworkBinding::RemoveBinding(binding);
#ifdef OS_OSX
		[pool release];
#endif
	}
	void HTTPClientBinding::Cancel(const ValueList& args, SharedValue result)
	{
		this->Set("connected",Value::NewBool(false));
	}
	void HTTPClientBinding::GetHeader(const ValueList& args, SharedValue result)
	{
		if (this->response)
		{
			std::string name = args.at(0)->ToString();
			if (this->response->has(name))
			{
				result->SetString(this->response->get(name).c_str());
			}
			else
			{
				result->SetNull();
			}
		}
	}
}
