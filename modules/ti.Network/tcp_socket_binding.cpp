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
	TCPSocketBinding::TCPSocketBinding(Host* ti_host, std::string host, int port) :
		ti_host(ti_host), host(host), port(port), opened(false), 
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
			this->reactor.stop();
			this->socket.close();
		}
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
		std::string eprefix = "Connect exception: ";
		if (this->opened)
		{
			throw ValueException::FromString(eprefix + "Socket is already open");
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
			throw ValueException::FromString(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			throw ValueException::FromString(eprefix + e.what());
		}
		catch(...)
		{
			throw ValueException::FromString(eprefix + "Unknown exception");
		}
	}
	void TCPSocketBinding::OnRead(const Poco::AutoPtr<ReadableNotification>& n)
	{
		// if we have no listeners, just bail...
		if (this->onRead==NULL && this->onReadComplete==NULL)
		{
			return;
		}

		std::string eprefix = "TCPSocketBinding::OnRead: ";
		try
		{
			char data[BUFFER_SIZE];
			int size = socket.receiveBytes(&data,BUFFER_SIZE);
			if (size <= 0)
			{
				if (this->onReadComplete)
				{
					SharedPtr<ValueList> a(new ValueList);
					ti_host->InvokeMethodOnMainThread(*this->onReadComplete,a);
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
			ti_host->InvokeMethodOnMainThread(*this->onRead,a);
		}
		catch(ValueException& e)
		{
			std::cerr << eprefix << e.GetValue()->DisplayString() << std::endl;
		}
		catch(std::exception &e)
		{
			std::cerr << eprefix << e.what() << std::endl;
		}
		catch(...)
		{
			std::cerr << eprefix << "Unknown exception" << std::endl;
		}
	}
	void TCPSocketBinding::OnWrite(const Poco::AutoPtr<WritableNotification>& n)
	{
		if (this->onWrite == NULL)
		{
			return;
		}
		SharedPtr<ValueList> a(new ValueList);
		ti_host->InvokeMethodOnMainThread(*this->onWrite,a);
	}
	void TCPSocketBinding::OnTimeout(const Poco::AutoPtr<TimeoutNotification>& n)
	{
		if (this->onTimeout == NULL)
		{
			return;
		}
		SharedPtr<ValueList> a(new ValueList);
		ti_host->InvokeMethodOnMainThread(*this->onTimeout,a);
	}
	void TCPSocketBinding::Write(const ValueList& args, SharedValue result)
	{
		std::string eprefix = "TCPSocketBinding::Write: ";
		if (!this->opened)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open");
		}

		try
		{
			std::string buf = args.at(0)->ToString();
			int count = this->socket.sendBytes(buf.c_str(),buf.length());
			result->SetInt(count);
		}
		catch(Poco::Exception &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}

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

