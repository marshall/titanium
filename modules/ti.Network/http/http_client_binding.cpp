/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifdef OS_OSX	//For some reason, 10.5 was fine with Cocoa headers being last, but 10.4 balks.
#import <Cocoa/Cocoa.h>
#endif

#include <kroll/kroll.h>
#include "http_client_binding.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include "../network_binding.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/MultipartWriter.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/File.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Zip/Compress.h"

namespace ti
{
	HTTPClientBinding::HTTPClientBinding(Host* host) : 
		host(host),global(host->GetGlobalObject()),
		thread(NULL),response(NULL),async(true),filestream(NULL)
	{
		this->SetMethod("abort",&HTTPClientBinding::Abort);
		this->SetMethod("open",&HTTPClientBinding::Open);
		this->SetMethod("setRequestHeader",&HTTPClientBinding::SetRequestHeader);
		this->SetMethod("send",&HTTPClientBinding::Send);
		this->SetMethod("sendFile",&HTTPClientBinding::SendFile);
		this->SetMethod("sendDir",&HTTPClientBinding::SendDir);
		this->SetMethod("getResponseHeader",&HTTPClientBinding::GetResponseHeader);

		SET_INT_PROP("readyState",0)
		SET_INT_PROP("UNSENT",0)
		SET_INT_PROP("OPENED",1)
		SET_INT_PROP("HEADERS_RECEIVED",2)
		SET_INT_PROP("LOADING",3)
		SET_INT_PROP("DONE",4)
		SET_NULL_PROP("responseText")
		SET_NULL_PROP("responseXML")
		SET_NULL_PROP("status")
		SET_NULL_PROP("statusText")
		SET_BOOL_PROP("connected",false)
		SET_NULL_PROP("onreadystatechange")
		SET_NULL_PROP("ondatastream")

		this->self = Value::NewObject(this);
	}
	HTTPClientBinding::~HTTPClientBinding()
	{
		if (this->thread!=NULL)
		{
			delete this->thread;
			this->thread = NULL;
		}
		if (this->filestream!=NULL)
		{
			delete this->filestream;
			this->filestream = NULL;
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
		std::ostringstream ostr;
		int max_redirects = 5;
		std::string url = binding->url;
		for (int x=0;x<max_redirects;x++)
		{
			Poco::URI uri(url);
			std::string path(uri.getPathAndQuery());
			if (path.empty()) path = "/";
			binding->Set("connected",Value::NewBool(true));
			Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
			
			std::string method = binding->method;
			if (method.empty())
			{
				method = Poco::Net::HTTPRequest::HTTP_GET;
			}
			
			if (!binding->dirstream.empty())
			{
				method = Poco::Net::HTTPRequest::HTTP_POST;
//				binding->headers["Content-Type"]="multipart/form-data; boundary=TitaniumRocks";
				binding->headers["Content-Type"]="application/zip";
			}

			Poco::Net::HTTPRequest req(method, path, Poco::Net::HTTPMessage::HTTP_1_1);
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
			//FIXME: implement cookies
			//FIXME: implement username/pass
			//FIXME: use proxy settings of system

			// set the headers
			if (binding->headers.size()>0)
			{
				std::map<std::string,std::string>::iterator i = binding->headers.begin();
				while(i!=binding->headers.end())
				{
					req.set((*i).first, (*i).second);
					i++;
				}
			}
			
			std::string data;
			int content_len = 0;

			if (!binding->dirstream.empty())
			{
				std::ostringstream ostr;
				Poco::Zip::Compress compressor(ostr,false);
				Poco::Path path(binding->dirstream);
				compressor.addRecursive(path);
				compressor.close();
				data = ostr.str();
				content_len = data.length();
			}
			else if (binding->datastream.empty())
			{
				data = binding->datastream;
				content_len = data.length();
			}

			// determine the content length
			if (!data.empty())
			{
				std::ostringstream l;
				l << content_len;
				req.set("Content-Length",l.str().c_str());
			}
			else if (!binding->filename.empty())
			{
				Poco::File f(binding->filename);
				std::ostringstream l;
				l << f.getSize();
				req.set("Content-Length",l.str().c_str());
			}
			
			// send and stream output
			std::ostream& out = session.sendRequest(req);
			
			// write out the data
			if (!data.empty())
			{
				out << data;
			}
			else if (binding->filestream)
			{
				Poco::StreamCopier::copyStream(*binding->filestream,out);
			}

			Poco::Net::HTTPResponse res;
			std::istream& rs = session.receiveResponse(res);
			int total = res.getContentLength();
#ifdef DEBUG
			std::cout << "HTTPClientBinding:: response length received = " << total << " - ";
			std::cout << res.getStatus() << " " << res.getReason() << std::endl;
#endif
			binding->Set("status",Value::NewInt(res.getStatus()));
			binding->Set("statusText",Value::NewString(res.getReason().c_str()));

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
			binding->response = &res;
			binding->ChangeState(2); // headers received
			binding->ChangeState(3); // loading

			int count = 0;
			char buf[8096];
			
			SharedBoundMethod streamer;
			SharedValue sv = binding->Get("ondatastream");
			if (sv->IsMethod())
			{
				streamer = sv->ToMethod()->Get("apply")->ToMethod();
			}
			
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
						if (streamer.get())
						{
							ValueList args;
							SharedBoundList list = new StaticBoundList();
							
							args.push_back(binding->self); // reference to us
							args.push_back(Value::NewList(list));
							
							list->Append(Value::NewInt(count)); // total count
							list->Append(totalValue); // total size
							list->Append(Value::NewObject(new Blob(buf,c))); // buffer
							list->Append(Value::NewInt(c)); // buffer length
						
							binding->host->InvokeMethodOnMainThread(streamer,args);
						}
						else
						{
							ostr << buf;
						}
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
		std::string data = ostr.str();
		if (!data.empty())
		{
			binding->Set("responseText",Value::NewString(data.c_str()));
		}
		binding->Set("connected",Value::NewBool(false));
		binding->ChangeState(4); // closed
		NetworkBinding::RemoveBinding(binding);
#ifdef OS_OSX
		[pool release];
#endif
	}
	void HTTPClientBinding::Send(const ValueList& args, SharedValue result)
	{
		if (this->Get("connected")->ToBool())
		{
			throw ValueException::FromString("already connected");
		}
		if (args.size()==1)
		{
			// can be a string of data or a file
			SharedValue v = args.at(0);
			if (v->IsObject())
			{
				// probably a file
				SharedBoundObject obj = v->ToObject();
				this->datastream = obj->Get("toString")->ToMethod()->Call()->ToString();
			}
			else if (v->IsString())
			{
				this->datastream = v->ToString();
			}
			else
			{
				throw ValueException::FromString("unknown data type");
			}
		}
		this->thread = new Poco::Thread();
		this->thread->start(&HTTPClientBinding::Run,(void*)this);
		if (!this->async)
		{
			this->thread->join();
		}
	}
	void HTTPClientBinding::SendFile(const ValueList& args, SharedValue result)
	{
		if (this->Get("connected")->ToBool())
		{
			throw ValueException::FromString("already connected");
		}
		if (args.size()==1)
		{
			// can be a string of data or a file
			SharedValue v = args.at(0);
			if (v->IsObject())
			{
				// probably a file
				SharedBoundObject obj = v->ToObject();
				if (obj->Get("isFile")->IsMethod())
				{
					std::string file = obj->Get("toString")->ToMethod()->Call()->ToString();
					this->filestream = new Poco::FileInputStream(file);
					Poco::Path p(file);
					this->filename = p.getFileName();
				}
				else
				{
					this->datastream = obj->Get("toString")->ToMethod()->Call()->ToString();
				}
			}
			else if (v->IsString())
			{
				this->filestream = new Poco::FileInputStream(v->ToString());
			}
			else
			{
				throw ValueException::FromString("unknown data type");
			}
		}
		this->thread = new Poco::Thread();
		this->thread->start(&HTTPClientBinding::Run,(void*)this);
		if (!this->async)
		{
			this->thread->join();
		}
	}
	void HTTPClientBinding::SendDir(const ValueList& args, SharedValue result)
	{
		if (this->Get("connected")->ToBool())
		{
			throw ValueException::FromString("already connected");
		}
		if (args.size()==1)
		{
			// can be a string of data or a file
			SharedValue v = args.at(0);
			if (v->IsObject())
			{
				// probably a file
				SharedBoundObject obj = v->ToObject();
				if (obj->Get("isFile")->IsMethod())
				{
					this->dirstream = obj->Get("toString")->ToMethod()->Call()->ToString();
				}
				else
				{
					this->dirstream = obj->Get("toString")->ToMethod()->Call()->ToString();
				}
			}
			else if (v->IsString())
			{
				this->dirstream = v->ToString();
			}
			else
			{
				throw ValueException::FromString("unknown data type");
			}
		}
		this->thread = new Poco::Thread();
		this->thread->start(&HTTPClientBinding::Run,(void*)this);
		if (!this->async)
		{
			this->thread->join();
		}
	}
	void HTTPClientBinding::Abort(const ValueList& args, SharedValue result)
	{
		SET_BOOL_PROP("connected",false)
	}
	void HTTPClientBinding::Open(const ValueList& args, SharedValue result)
	{
		if (args.size()<2)
		{
			throw ValueException::FromString("invalid arguments");
		}
		this->method = args.at(0)->ToString();
		this->url = args.at(1)->ToString();
		if (args.size()>=3)
		{
			GET_BOOL_PROP(args.at(2),this->async)
		}
		if (args.size()>=4)
		{
			this->user = args.at(3)->ToString();
		}
		if (args.size()>4)
		{
			this->password = args.at(4)->ToString();
		}
		this->ChangeState(1); // opened
	}
	void HTTPClientBinding::SetRequestHeader(const ValueList& args, SharedValue result)
	{
		const char *key = args.at(0)->ToString();
		const char *value = args.at(1)->ToString();
		this->headers[std::string(key)]=std::string(value);
	}
	void HTTPClientBinding::GetResponseHeader(const ValueList& args, SharedValue result)
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
	void HTTPClientBinding::ChangeState(int readyState)
	{
		SET_INT_PROP("readyState",readyState) 
		SharedValue v = this->Get("onreadystatechange");
		if (v->IsMethod())
		{
			try
			{
				SharedBoundMethod m = v->ToMethod()->Get("call")->ToMethod();
				ValueList args;
				args.push_back(this->self);
				this->host->InvokeMethodOnMainThread(m,args);
			}
			catch (std::exception &ex)
			{
				std::cerr << "Exception calling readyState. Exception: " << ex.what() << std::endl;
			}
		}
	}
}
