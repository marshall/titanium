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
		this->Set("connected",Value::NewBool(false));
		this->SetMethod("connect",&IRCClientBinding::Connect);
		this->SetMethod("disconnect",&IRCClientBinding::Disconnect);
		this->SetMethod("send",&IRCClientBinding::Send);
		this->SetMethod("sendPrivate",&IRCClientBinding::SendPrivate);
		this->SetMethod("setNick",&IRCClientBinding::SetNick);
		this->SetMethod("getNick",&IRCClientBinding::GetNick);
		this->SetMethod("getUsers",&IRCClientBinding::GetUsers);
		this->SetMethod("join",&IRCClientBinding::Join);
		this->SetMethod("unjoin",&IRCClientBinding::Unjoin);
		this->SetMethod("isOp",&IRCClientBinding::IsOp);
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
				binding->host->InvokeMethodOnMainThread(binding->callback,args);
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
			this->irc.notice((char*)channel,(char*)msg);
		}
	}
	void IRCClientBinding::SendPrivate(const ValueList& args, SharedValue result)
	{
		bool connected = this->Get("connected")->ToBool();
		if (connected)
		{
			const char *channel = args.at(0)->ToString();
			const char *msg = args.at(1)->ToString();
			this->irc.privmsg((char*)channel,(char*)msg);
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
