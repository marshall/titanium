/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <Poco/NObserver.h>
#include <kroll/kroll.h>


namespace ti
{
	TCPSocketBinding::TCPSocketBinding(std::string host, int port) : 
		host(host), port(port), opened(false), callback(0)
	{
		this->SetMethod("connect",&TCPSocketBinding::Connect);
		this->SetMethod("close",&TCPSocketBinding::Close);
		this->SetMethod("write",&TCPSocketBinding::Write);
	}
	TCPSocketBinding::~TCPSocketBinding()
	{
		if (this->opened)
		{
			this->reactor.stop();
			this->socket.close();
		}
		KR_DECREF(this->callback);
	}
	void TCPSocketBinding::Connect(const ValueList& args, Value *result)
	{
		if (this->opened)
		{
			throw new Value("socket is already open");
			return;
		}
		this->callback = args.at(0)->ToMethod();
		KR_ADDREF(this->callback);
		try
		{
			SocketAddress a(this->host.c_str(),this->port);
			this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, ReadableNotification>(*this, &TCPSocketBinding::OnRead));
			this->socket.connectNB(a);
			this->thread.start(this->reactor);
			this->opened = true;
		}
		catch(Poco::IOException &e)
		{
			std::cout << "exception:" << e.displayText() << std::endl;
			std::string msg = e.displayText();
			throw new Value(msg);
		}
		catch(std::exception &e)
		{
			std::string msg("connect exception: ");
			msg+=e.what();
			throw new Value(msg);
		}
		catch(...)
		{
			throw new Value("unknown exception caught in connect");
		}
	}
	void TCPSocketBinding::OnRead(const Poco::AutoPtr<ReadableNotification>& n)
	{
		static int bufsize = 1024;  // choose a reasonable size to send back to JS
		char data[bufsize];
		try
		{
			int size = socket.receiveBytes(&data,bufsize);
			if (size <= 0) return;
			data[size]='\0';
			std::string s(data);
			Value* value = new Value(s);
			ValueList* args = new ValueList;
			args->push_back(value);
			kroll::InvokeMethodOnMainThread(this->callback,args);
			delete args;
			KR_DECREF(value);
		}
		catch(...)
		{
			std::cerr << "Network error TCPSocketBinding::OnRead" << std::endl;
		}
	}
	void TCPSocketBinding::Write(const ValueList& args, Value *result)
	{
		if (!this->opened)
		{
			Value *exception = new Value("socket is closed");
			throw exception;
			return;
		}
		std::string buf = args.at(0)->ToString();
		int count = this->socket.sendBytes(buf.c_str(),buf.length());
		std::cout << "count = " << count << std::endl;
		result->Set(count);
	}
	void TCPSocketBinding::Close(const ValueList& args, Value *result)
	{
		if (this->opened)
		{
			this->socket.close();
			this->opened = false;
			return;
		}
	}
}

