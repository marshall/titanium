/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "irc_client_binding.h"
#include <cstring>

#ifdef OS_OSX
  #import <Cocoa/Cocoa.h>
#endif


namespace ti
{
	IRCClientBinding::IRCClientBinding(Host* host) : 
		host(host), global(host->GetGlobalObject()), thread(NULL)
	{
		/**
		 * @tiapi(property=True,type=boolean,name=Network.IRC.connected,since=0.2) returns true if connected
		 */
		this->Set("connected",Value::NewBool(false));
		/**
		 * @tiapi(method=True,name=Network.IRC.connect,since=0.2) connect the IRC connection
		 */
		this->SetMethod("connect",&IRCClientBinding::Connect);
		/**
		 * @tiapi(method=True,name=Network.IRC.disconnect,since=0.2) disconnect the IRC connection
		 */
		this->SetMethod("disconnect",&IRCClientBinding::Disconnect);
		/**
		 * @tiapi(method=True,name=Network.IRC.send,since=0.2) send data on the IRC connection
		 * @tiarg(for=Network.IRC.send,type=string,name=channel) channel
		 * @tiarg(for=Network.IRC.send,type=string,name=message) message
		 */
		this->SetMethod("send",&IRCClientBinding::Send);
		/**
		 * @tiapi(method=True,name=Network.IRC.setNick,since=0.2) set the nick name for the connection
		 * @tiarg(for=Network.IRC.setNick,type=string,name=nick) nickname
		 */
		this->SetMethod("setNick",&IRCClientBinding::SetNick);
		/**
		 * @tiapi(method=True,name=Network.IRC.getNick,since=0.2) get the nick name for the connection
		 * @tiresult(for=Network.IRC.getNick,type=string) returns nickname
		 */
		this->SetMethod("getNick",&IRCClientBinding::GetNick);
		/**
		 * @tiapi(method=True,name=Network.IRC.getUsers,since=0.2) get a list of users for the channel
		 * @tiresult(for=Network.IRC.getUsers,type=list) returns list of users
		 */
		this->SetMethod("getUsers",&IRCClientBinding::GetUsers);
		/**
		 * @tiapi(method=True,name=Network.IRC.join,since=0.2) join a channel
		 * @tiarg(for=Network.IRC.join,type=string,name=channel) channel
		 */
		this->SetMethod("join",&IRCClientBinding::Join);
		/**
		 * @tiapi(method=True,name=Network.IRC.unjoin,since=0.2) unjoin from a channel
		 * @tiarg(for=Network.IRC.unjoin,type=string,name=channel) channel
		 */
		this->SetMethod("unjoin",&IRCClientBinding::Unjoin);
		/**
		 * @tiapi(method=True,name=Network.IRC.isOp,since=0.2) returns true if the user is an operator
		 * @tiresult(for=Network.IRC.isOp,type=boolean) true if user is operator
		 */
		this->SetMethod("isOp",&IRCClientBinding::IsOp);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IRC.isVoice,since=0.2) returns true if the user has voice
		 * @tiresult(for=Network.IRC.isVoice,type=boolean) true if user has voice
		 */
		this->SetMethod("isVoice",&IRCClientBinding::IsVoice);

		// NULL is how we hook all commands (wildcard)
		this->irc.hook_irc_command(NULL,&IRCClientBinding::Callback,(void*)this);
		this->thread = new Poco::Thread();
	}
	IRCClientBinding::~IRCClientBinding()
	{
		bool connected = this->Get("connected")->ToBool();
		if (connected)
		{
			this->irc.quit("Leaving"); 
		}
		if (this->thread!=NULL)
		{
			delete this->thread;
			this->thread = NULL;
		}
	}
	int IRCClientBinding::Callback(char *irc_command, char* param, irc_reply_data* data, void* conn, void *pd)
	{
#ifdef DEBUG
		std::cout << "Received: " << param << std::endl;
#endif
		IRCClientBinding *binding = (IRCClientBinding*)pd;
		if (!binding->callback.isNull())
		{
			ValueList args;
			args.push_back(irc_command ? Value::NewString(irc_command) : Value::Null);
			args.push_back(param ? Value::NewString(param) : Value::Null);
			args.push_back(data->target ? Value::NewString(data->target): Value::Null);
			args.push_back(data->nick ? Value::NewString(data->nick): Value::Null);

			try
			{
				binding->host->InvokeMethodOnMainThread(binding->callback,args,false);
			}
			catch(std::exception &e)
			{
				std::cerr << "Caught exception dispatching IRC callback: " << irc_command << ", Error: " << e.what() << std::endl;
			}
		}
		return 0;
	}
	void IRCClientBinding::Run (void* p)
	{
#ifdef OS_OSX
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif
		IRC *irc = (IRC*)p;
		irc->message_loop();
#ifdef OS_OSX
		[pool release];
#endif
	}
	void IRCClientBinding::GetUsers(const ValueList& args, SharedValue result)
	{
		const char *channel = args.at(0)->ToString();
		SharedBoundList list = new StaticBoundList();
		channel_user* cu = irc.get_users();
		while(cu)
		{
			if (!strcmp(cu->channel,(char*)channel) && cu->nick && strlen(cu->nick)>0)
			{
				SharedBoundObject entry = new StaticBoundObject();
				entry->Set("name",Value::NewString(cu->nick));
				entry->Set("operator",Value::NewBool(cu->flags & IRC_USER_OP));
				entry->Set("voice",Value::NewBool(cu->flags & IRC_USER_VOICE));
				list->Append(Value::NewObject(entry));
			}
			cu = cu->next;
		}
		result->SetList(list);
	}
	void IRCClientBinding::Connect(const ValueList& args, SharedValue result)
	{
		//TODO: check to make sure not connected already
		//TODO: check args
		std::string hostname = args.at(0)->ToString();
		int port = args.at(1)->ToInt();
		std::string nick = args.at(2)->ToString(); 
		std::string name = args.at(3)->ToString();
		std::string user = args.at(4)->ToString();
		std::string pass = args.at(5)->ToString();
		this->callback = args.at(6)->ToMethod();

		//char* server, int port, char* nick, char* user, char* name, char* pass
		this->irc.start((char*)hostname.c_str(),
						port,
						(char*)nick.c_str(),
						(char*)user.c_str(),
						(char*)name.c_str(),
						(char*)pass.c_str());

		this->Set("connected",Value::NewBool(true));
		this->thread->start(&IRCClientBinding::Run,&irc);
	}
	void IRCClientBinding::Disconnect(const ValueList& args, SharedValue result)
	{
		bool connected = this->Get("connected")->ToBool();
		if (connected)
		{
			const char *msg = args.size()>0 ? args.at(0)->ToString() : "Leaving";
			this->irc.quit((char*)msg); 
			this->Set("connected",Value::NewBool(false));
		}
	}
	void IRCClientBinding::Send(const ValueList& args, SharedValue result)
	{
		bool connected = this->Get("connected")->ToBool();
		if (connected)
		{
			const char *channel = args.at(0)->ToString();
			const char *msg = args.at(1)->ToString();
#ifdef DEBUG
			std::cout << "sending IRC: " << channel << " => " << msg << std::endl;
#endif
			std::string cmd(msg);
			size_t pos = std::string::npos;
			// this is a little raw, we need to probably refactor
			// this to something more sane...
			if ((pos = cmd.find("/nick "))==0)
			{
				this->irc.nick((char*)cmd.substr(6).c_str());
			}
			else
			{
				this->irc.privmsg((char*)channel,(char*)msg);
			}
		}
	}
	void IRCClientBinding::SetNick(const ValueList& args, SharedValue result)
	{
		const char *nick = args.at(0)->ToString();
		this->irc.nick((char*)nick);
	}
	void IRCClientBinding::GetNick(const ValueList& args, SharedValue result)
	{
		std::string nick = this->irc.current_nick();
		result->SetString(nick);
	}
	void IRCClientBinding::Join(const ValueList& args, SharedValue result)
	{
		bool connected = this->Get("connected")->ToBool();
		if (connected)
		{
			const char *channel = args.at(0)->ToString();
#ifdef DEBUG
			std::cout << "JOIN " << channel << std::endl;
#endif
			this->irc.join((char*)channel);
		}
	}
	void IRCClientBinding::Unjoin(const ValueList& args, SharedValue result)
	{
		bool connected = this->Get("connected")->ToBool();
		if (connected)
		{
			const char *channel = args.at(0)->ToString();
			this->irc.part((char*)channel);
		}
	}
	void IRCClientBinding::IsOp(const ValueList& args, SharedValue result)
	{
		//TODO:
	}
	void IRCClientBinding::IsVoice(const ValueList& args, SharedValue result)
	{
		//TODO:
	}
}
