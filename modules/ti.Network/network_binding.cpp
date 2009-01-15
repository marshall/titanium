/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "network_binding.h"
#include "tcp_socket_binding.h"
#include <kroll/kroll.h>

namespace ti
{
	NetworkBinding::NetworkBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
		this->SetMethod("createTCPSocket",&NetworkBinding::Create);
	}
	NetworkBinding::~NetworkBinding()
	{
		KR_DECREF(global);
	}
	void NetworkBinding::Create(const ValueList& args, Value *result)
	{
		BoundObject *tcp = new TCPSocketBinding(args.at(0)->ToString(), args.at(1)->ToInt());
		result->Set(tcp);
		KR_DECREF(tcp);

		// SocketAddress addr("localhost",80);
		// StreamSocket *sock = new StreamSocket();
		// sock->connect(addr);
		// std::string buf("GET / HTTP/1.0\r\n\r\n");
		// int count = sock->sendBytes(buf.c_str(),buf.length());
		// std::cout << "count = " << count << std::endl;
		// char rbuf[4000];
		// int rcount = sock->receiveBytes(&rbuf,4000);
		// rbuf[rcount]='\0';
		// std::cout << "received = " << rbuf << std::endl;
		// sock->close();
		// delete sock;
	}
}
