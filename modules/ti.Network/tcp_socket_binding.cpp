/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <Poco/NObserver.h>
#include <kroll/kroll.h>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	TCPSocketBinding::TCPSocketBinding(std::string host, int port) :
		host(host), port(port), opened(false), 
		onRead(0), onWrite(0), onTimeout(0), onReadComplete(0)
	{
		// methods
		this->SetMethod("connect",&TCPSocketBinding::Connect);
		this->SetMethod("close",&TCPSocketBinding::Close);
		this->SetMethod("write",&TCPSocketBinding::Write);
		this->SetMethod("isClosed",&TCPSocketBinding::IsClosed);

		// event handler callbacks
		this->SetMethod("onRead",&TCPSocketBinding::SetOnRead);
		this->SetMethod("onWrite",&TCPSocketBinding::SetOnWrite);
		this->SetMethod("onTimeout",&TCPSocketBinding::SetOnTimeout);
		this->SetMethod("onReadComplete",&TCPSocketBinding::SetOnReadComplete);

		// our reactor event handlers
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, ReadableNotification>(*this, &TCPSocketBinding::OnRead));
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, WritableNotification>(*this, &TCPSocketBinding::OnWrite));
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, TimeoutNotification>(*this, &TCPSocketBinding::OnTimeout));
	}
	TCPSocketBinding::~TCPSocketBinding()
	{
		KR_DUMP_LOCATION
		
		if (this->onTimeout)
		{
			delete this->onTimeout;
		}
		if (this->onRead)
		{
			delete this->onRead;
		}
		if (this->onWrite)
		{
			delete this->onWrite;
		}
		if (this->onReadComplete)
		{
			delete this->onReadComplete;
		}
		if (this->opened)
		{
			std::cout << "before ~TCPSocketBinding reactor.stop" << std::endl;
			this->reactor.stop();
			std::cout << "before ~TCPSocketBinding socket.close" << std::endl;
			this->socket.close();
		}
		std::cout << "exiting ~TCPSocketBinding" << std::endl;
	}
	void TCPSocketBinding::SetOnRead(const ValueList& args, SharedValue result)
	{
		if (this->onRead)
		{
			delete this->onRead;
		}
		this->onRead = new SharedBoundMethod(args.at(0)->ToMethod());
	}
	void TCPSocketBinding::SetOnWrite(const ValueList& args, SharedValue result)
	{
		if (this->onWrite)
		{
			delete this->onWrite;
		}
		this->onWrite = new SharedBoundMethod(args.at(0)->ToMethod());
	}
	void TCPSocketBinding::SetOnTimeout(const ValueList& args, SharedValue result)
	{
		if (this->onTimeout)
		{
			delete this->onTimeout;
		}
		this->onTimeout = new SharedBoundMethod(args.at(0)->ToMethod());
	}
	void TCPSocketBinding::SetOnReadComplete(const ValueList& args, SharedValue result)
	{
		if (this->onReadComplete)
		{
			delete this->onReadComplete;
		}
		this->onReadComplete = new SharedBoundMethod(args.at(0)->ToMethod());
	}
	void TCPSocketBinding::IsClosed(const ValueList& args, SharedValue result)
	{
		return result->SetBool(!this->opened);
	}
	void TCPSocketBinding::Connect(const ValueList& args, SharedValue result)
	{
		if (this->opened)
		{
			throw Value::NewString("socket is already open");
		}
		try
		{
			SocketAddress a(this->host.c_str(),this->port);
			this->socket.connectNB(a);
			this->thread.start(this->reactor);
			this->opened = true;
			result->SetBool(true);
		}
		catch(Poco::IOException &e)
		{
			std::string msg = e.displayText();
			throw Value::NewString(msg);
		}
		catch(std::exception &e)
		{
			std::string msg("connect exception: ");
			msg+=e.what();
			throw Value::NewString(msg);
		}
		catch(...)
		{
			throw Value::NewString("unknown exception caught in connect");
		}
	}
	void TCPSocketBinding::OnRead(const Poco::AutoPtr<ReadableNotification>& n)
	{
		// if we have no listeners, just bail...
		if (this->onRead==NULL && this->onReadComplete==NULL)
		{
			return;
		}
		try
		{
			char data[BUFFER_SIZE];
			int size = socket.receiveBytes(&data,BUFFER_SIZE);
			if (size <= 0)
			{
				if (this->onReadComplete)
				{
					SharedPtr<ValueList> a(new ValueList);
					InvokeMethodOnMainThread(*this->onReadComplete,a);
				}
				return;
			}
			// do this after we read so that we can deal with 
			// on read complete
			if (this->onRead == NULL)
			{
				return;
			}
			data[size]='\0';
			std::string s(data);
			SharedValue value = Value::NewString(s);
			ValueList *args = new ValueList;
			args->push_back(value);
			SharedPtr<ValueList> a(args);
			InvokeMethodOnMainThread(*this->onRead,a);
		}
		catch(std::exception &e)
		{
			std::cerr << "Network error TCPSocketBinding::OnRead: " << e.what() << std::endl;
		}
		catch(SharedValue &e)
		{
			std::cerr << "Network error TCPSocketBinding::OnRead: " << e->ToString() << std::endl;
		}
		catch(...)
		{
			std::cerr << "Network error TCPSocketBinding::OnRead" << std::endl;
		}
	}
	void TCPSocketBinding::OnWrite(const Poco::AutoPtr<WritableNotification>& n)
	{
		if (this->onWrite == NULL)
		{
			return;
		}
		SharedPtr<ValueList> a(new ValueList);
		InvokeMethodOnMainThread(*this->onWrite,a);
	}
	void TCPSocketBinding::OnTimeout(const Poco::AutoPtr<TimeoutNotification>& n)
	{
		if (this->onTimeout == NULL)
		{
			return;
		}
		SharedPtr<ValueList> a(new ValueList);
		InvokeMethodOnMainThread(*this->onTimeout,a);
	}
	void TCPSocketBinding::Write(const ValueList& args, SharedValue result)
	{
		if (!this->opened)
		{
			throw Value::NewString("socket is closed");
		}
		std::string buf = args.at(0)->ToString();
		int count = this->socket.sendBytes(buf.c_str(),buf.length());
		result->SetInt(count);
	}
	void TCPSocketBinding::Close(const ValueList& args, SharedValue result)
	{
		if (this->opened)
		{
			this->opened = false;
			this->reactor.stop();
			this->socket.close();
			result->SetBool(true);
		}
		else
		{
			result->SetBool(false);
		}
	}
}

