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
#include <fstream>
#include "../network_binding.h"
#include <Poco/Buffer.h>
#include <Poco/Net/MultipartWriter.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/File.h>
#include <Poco/Timespan.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Zip/Compress.h>
#include <Poco/Zip/ZipCommon.h>

#ifdef verify
#define __verify verify
#undef verify
#endif

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>

#ifdef __verify
#define verify __verify
#undef __verify
#endif

namespace ti
{
	bool HTTPClientBinding::initialized = false;
	
	HTTPClientBinding::HTTPClientBinding(Host* host, std::string path) :
		host(host),modulePath(path),global(host->GetGlobalObject()),
		thread(NULL),response(NULL),async(true),filestream(NULL),
		timeout(30000)
	{
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.abort,since=0.3) abort an in progress connection
		 */
		this->SetMethod("abort",&HTTPClientBinding::Abort);
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.open,since=0.3) open the connection
		 */
		this->SetMethod("open",&HTTPClientBinding::Open);
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.setRequestHeader,since=0.3) set a request header
		 * @tiarg(for=Network.HTTPClient.setRequestHeader,name=key,type=string) request key
		 * @tiarg(for=Network.HTTPClient.setRequestHeader,name=value,type=string) request value
		 */
		this->SetMethod("setRequestHeader",&HTTPClientBinding::SetRequestHeader);
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.send,since=0.3) send data
		 * @tiarg(for=Network.HTTPClient.send,type=string,name=data) data to send
		 */
		this->SetMethod("send",&HTTPClientBinding::Send);
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.sendFile,since=0.3) send file contents as body content
		 * @tiarg(for=Network.HTTPClient.sendFile,type=string,name=data) file path to send
		 */
		this->SetMethod("sendFile",&HTTPClientBinding::SendFile);
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.sendDir,since=0.3) send a directory as a zipped body
		 * @tiarg(for=Network.HTTPClient.sendDir,type=string,name=data) directory with contents to send
		 */
		this->SetMethod("sendDir",&HTTPClientBinding::SendDir);
		/**
		 * @tiapi(method=True,name=Network.HTTPClient.getResponseHeader,since=0.3) abort an in progress connection
		 * @tiarg(for=Network.HTTPClient.getResponseHeader,type=string,name=name) the response header name
		 * @tiresult(for=Network.HTTPClient.getResponseHeader,type=string) returns the response header by name
		 */
		this->SetMethod("getResponseHeader",&HTTPClientBinding::GetResponseHeader);

		/**
		 * @tiapi(method=True,name=Network.HTTPClient.setTimeout,since=0.4) set timeout when request will be aborted if not completed
		 * @tiarg(for=Network.HTTPClient.setTimeout,type=integer,name=timeout) value in milliseconds
		 */
		this->SetMethod("setTimeout",&HTTPClientBinding::SetTimeout);

		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.readyState,since=0.3) get the ready state property for the connection
		 */
		SET_INT_PROP("readyState",0)

		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.UNSENT,since=0.3) the UNSENT readyState property
		 */
		SET_INT_PROP("UNSENT",0)
		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.OPENED,since=0.3) the OPENED readyState property
		 */
		SET_INT_PROP("OPENED",1)
		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.HEADERS_RECEIVED,since=0.3) the HEADERS_RECEIVED readyState property
		 */
		SET_INT_PROP("HEADERS_RECEIVED",2)
		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.LOADING,since=0.3) the LOADING readyState property
		 */
		SET_INT_PROP("LOADING",3)
		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.DONE,since=0.3) the DONE readyState property
		 */
		SET_INT_PROP("DONE",4)

		/**
		 * @tiapi(property=True,type=string,name=Network.HTTPClient.responseText,since=0.3) returns the response as text
		 */
		SET_NULL_PROP("responseText")
		/**
		 * @tiapi(property=True,type=object,name=Network.HTTPClient.responseXML,since=0.3) returns the response as DOM
		 */
		SET_NULL_PROP("responseXML")
		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.status,since=0.3) returns the status code
		 */
		SET_NULL_PROP("status")
		/**
		 * @tiapi(property=True,type=string,name=Network.HTTPClient.statusText,since=0.3) returns the status text
		 */
		SET_NULL_PROP("statusText")
		/**
		 * @tiapi(property=True,type=integer,name=Network.HTTPClient.connected,since=0.3) return true if the connection is connected
		 */
		SET_BOOL_PROP("connected",false)

		/**
		 * @tiapi(property=True,type=method,name=Network.HTTPClient.onreadystatechange,since=0.3) set the ready state change function handler
		 */
		SET_NULL_PROP("onreadystatechange")
		/**
		 * @tiapi(property=True,type=method,name=Network.HTTPClient.ondatastream,since=0.3) set the function handler as the stream data is received
		 */
		SET_NULL_PROP("ondatastream")
		/**
		 * @tiapi(property=True,type=method,name=Network.HTTPClient.onsendstream,since=0.3) set the function handler as the stream data is sent
		 */
		SET_NULL_PROP("onsendstream")

		this->self = Value::NewObject(this);
	}
	HTTPClientBinding::~HTTPClientBinding()
	{
		KR_DUMP_LOCATION
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
		NetworkBinding::RemoveBinding(this);
	}
	void HTTPClientBinding::Run (void* p)
	{
#ifdef OS_OSX
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif
		HTTPClientBinding *binding = reinterpret_cast<HTTPClientBinding*>(p);

		PRINTD("HTTPClientBinding:: starting => " << binding->url);
		
		Poco::Net::HTTPResponse res;
		std::ostringstream ostr;
		int max_redirects = 5;
		int status;
		std::string url = binding->url;

		bool deletefile = false;

		for (int x=0;x<max_redirects;x++)
		{
			Poco::URI uri(url);
			std::string path(uri.getPathAndQuery());
			if (path.empty()) path = "/";
			binding->Set("connected",Value::NewBool(true));
			
			const std::string& scheme = uri.getScheme();
			SharedPtr<Poco::Net::HTTPClientSession> session;
			
			if (scheme=="https")
			{
				if (HTTPClientBinding::initialized==false)
				{
					HTTPClientBinding::initialized = true;
					SharedPtr<Poco::Net::InvalidCertificateHandler> ptrCert = new Poco::Net::AcceptCertificateHandler(false); 
					std::string rootpem = FileUtils::Join(binding->modulePath.c_str(),"rootcert.pem",NULL);
					Poco::Net::Context::Ptr ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE,"", "", rootpem, Poco::Net::Context::VERIFY_NONE, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
					Poco::Net::SSLManager::instance().initializeClient(0, ptrCert, ptrContext);
				}
				session = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
			}
			else if (scheme=="http")
			{
				session = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
			}
			else
			{
				//FIXME - we need to notify of unsupported error here
			}
			
			
			// set the timeout for the request
			Poco::Timespan to((long)binding->timeout,0L);
			session->setTimeout(to);

			std::string method = binding->method;
			if (method.empty())
			{
				method = Poco::Net::HTTPRequest::HTTP_GET;
			}

			if (!binding->dirstream.empty())
			{
				method = Poco::Net::HTTPRequest::HTTP_POST;
				binding->headers["Content-Type"]="application/zip";
			}

			Poco::Net::HTTPRequest req(method, path, Poco::Net::HTTPMessage::HTTP_1_1);
			const char* ua = binding->global->Get("userAgent")->IsString() ? binding->global->Get("userAgent")->ToString() : NULL;
			PRINTD("HTTPClientBinding:: userAgent = " << ua);
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
				std::string tmpdir = FileUtils::GetTempDirectory();
				Poco::File tmpPath (tmpdir);
				if (!tmpPath.exists()) {
					tmpPath.createDirectories();
				}
				std::ostringstream tmpfilename;
				tmpfilename << "ti";
				tmpfilename << rand();
				tmpfilename << ".zip";
				std::string fn(FileUtils::Join(tmpdir.c_str(),tmpfilename.str().c_str(),NULL));
				std::ofstream outfile(fn.c_str(), std::ios::binary|std::ios::out|std::ios::trunc);
				Poco::Zip::Compress compressor(outfile,true);
				Poco::Path path(binding->dirstream);
				compressor.addRecursive(path);
				compressor.close();
				outfile.close();
				deletefile = true;
				binding->filename = std::string(fn.c_str());
				binding->filestream = new Poco::FileInputStream(binding->filename);
			}

			if (!binding->datastream.empty())
			{
				data = binding->datastream;
				content_len = data.length();
			}

			// determine the content length
			if (!data.empty())
			{
				std::ostringstream l(std::stringstream::binary|std::stringstream::out);
				l << content_len;
				req.set("Content-Length",l.str());
			}
			else if (!binding->filename.empty())
			{
				Poco::File f(binding->filename);
				std::ostringstream l;
				l << f.getSize();
				const char *cl = l.str().c_str();
				content_len = atoi(cl);
				req.set("Content-Length", l.str());
			}

			// send and stream output
			std::ostream& out = session->sendRequest(req);

			// write out the data
			if (!data.empty())
			{
				out << data;
			}
			else if (binding->filestream)
			{
				// SharedKMethod sender;
				// SharedValue sv = binding->Get("onsendstream");
				// if (sv->IsMethod())
				// {
				// 	sender = sv->ToMethod()->Get("apply")->ToMethod();
				// }
				std::streamsize bufferSize = 8096;
				Poco::Buffer<char> buffer(bufferSize);
				std::streamsize len = 0;
				std::istream& istr = (*binding->filestream);
				istr.read(buffer.begin(), bufferSize);
				std::streamsize n = istr.gcount();
				int remaining = content_len;
				while (n > 0)
				{
					len += n;
					remaining -= n;
					out.write(buffer.begin(), n);
// 					if (sender.get())
// 					{
// 						try
// 						{
// #ifdef DEBUG
// 							std::cout << "ONSENDSTREAM = >> " << len <<" of " << content_len << std::endl;
// #endif
// 							ValueList args;
// 							SharedKList list = new StaticBoundList();
//
// 							args.push_back(binding->self); // reference to us
// 							args.push_back(Value::NewList(list));
//
// 							list->Append(Value::NewInt(len)); // bytes sent
// 							list->Append(Value::NewInt(content_len)); // total size
// 							list->Append(Value::NewInt(remaining)); // remaining
// 							binding->host->InvokeMethodOnMainThread(sender,args,true);
// 						}
// 						catch(std::exception &e)
// 						{
// 							std::cerr << "Caught exception dispatching HTTP callback on transmit, Error: " << e.what() << std::endl;
// 						}
// 						catch(...)
// 						{
// 							std::cerr << "Caught unknown exception dispatching HTTP callback on transmit" << std::endl;
// 						}
//					}
					if (istr)
					{
						istr.read(buffer.begin(), bufferSize);
						n = istr.gcount();
					}
					else n = 0;
				}
				binding->filestream->close();
			}

			std::istream& rs = session->receiveResponse(res);
			int total = res.getContentLength();
			status = res.getStatus();
			PRINTD("HTTPClientBinding:: response length received = " << total << " - " << status << " " << res.getReason());
			binding->Set("status",Value::NewInt(status));
			binding->Set("statusText",Value::NewString(res.getReason().c_str()));

			if (status == 301 || status == 302)
			{
				if (!res.has("Location"))
				{
					break;
				}
				url = res.get("Location");
				PRINTD("redirect to " << url);
				continue;
			}
			SharedValue totalValue = Value::NewInt(total);
			binding->response = &res;
			binding->ChangeState(2); // headers received
			binding->ChangeState(3); // loading

			int count = 0;
			char buf[8096];

			SharedKMethod streamer;
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
							SharedKList list = new StaticBoundList();

							args.push_back(binding->self); // reference to us
							args.push_back(Value::NewList(list));

							list->Append(Value::NewInt(count)); // total count
							list->Append(totalValue); // total size
							list->Append(Value::NewObject(new Blob(buf,c))); // buffer
							list->Append(Value::NewInt(c)); // buffer length

							binding->host->InvokeMethodOnMainThread(streamer,args,true);
						}
						else
						{
							ostr << buf;
						}
					}
				}
				catch(std::exception &e)
				{
					Logger logger = Logger::GetRootLogger();
					logger.Error("Caught exception dispatching HTTP callback, Error: %s",e.what());
				}
				catch(...)
				{
					Logger logger = Logger::GetRootLogger();
					logger.Error("Caught unknown exception dispatching HTTP callback");
				}
				if (rs.eof()) break;
			}
			break;
		}
		std::string data = ostr.str();
		if (!data.empty())
		{
#ifdef DEBUG
			if (status > 200)
			{
				PRINTD("RECEIVED = " << data);
			}
#endif
			binding->Set("responseText",Value::NewString(data.c_str()));
		}

		if (deletefile)
		{
			Poco::File f(binding->filename);
			f.remove();
		}

		binding->Set("connected",Value::NewBool(false));
		binding->ChangeState(4); // closed
		binding->response = NULL; // must be done after change state
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
				SharedKObject obj = v->ToObject();
				this->datastream = obj->Get("toString")->ToMethod()->Call()->ToString();
			}
			else if (v->IsString())
			{
				this->datastream = v->ToString();
			}
			else if (v->IsNull() || v->IsUndefined())
			{
				this->datastream = "";
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
			PRINTD("Waiting on HTTP Client thread to finish");
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
				SharedKObject obj = v->ToObject();
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
			PRINTD("Waiting on HTTP Client thread to finish");
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
				SharedKObject obj = v->ToObject();
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
			PRINTD("Waiting on HTTP Client thread to finish");
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
		if (this->response!=NULL)
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
		else
		{
			throw ValueException::FromString("no available response");
		}
	}
	void HTTPClientBinding::SetTimeout(const ValueList& args, SharedValue result)
	{
		this->timeout = args.at(0)->ToInt();
	}
	void HTTPClientBinding::ChangeState(int readyState)
	{
		SET_INT_PROP("readyState",readyState)
		SharedValue v = this->Get("onreadystatechange");
		if (v->IsMethod())
		{
			try
			{
				SharedKMethod m = v->ToMethod()->Get("call")->ToMethod();
				ValueList args;
				args.push_back(this->self);
				this->host->InvokeMethodOnMainThread(m,args,true);
			}
			catch (std::exception &ex)
			{
				Logger logger = Logger::GetRootLogger();
				logger.Error("Exception calling readyState. Exception: %s",ex.what());
			}
		}
	}
}
