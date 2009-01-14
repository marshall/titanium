/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TCP_SOCKET_BINDING_H_
#define _TCP_SOCKET_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <Poco/Thread.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>

using namespace Poco;
using namespace Poco::Net;

namespace ti
{
	class TCPSocketBinding : public StaticBoundObject
	{
	public:
		TCPSocketBinding(std::string host, int port);
	protected:
		virtual ~TCPSocketBinding();
	private:
		std::string host;
		int port;
		StreamSocket socket;
		SocketReactor reactor;
		Thread thread;
		bool opened;
		BoundMethod *callback;
		
		void Connect(const ValueList& args, Value *result);
		void Write(const ValueList& args, Value *result);
		void Close(const ValueList& args, Value *result);

		void OnRead(const Poco::AutoPtr<ReadableNotification>& n);
	};
}

#endif
