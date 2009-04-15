/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "database_binding.h"
#include "app_config.h"
#include "resultset_binding.h"

namespace ti
{
	DatabaseBinding::DatabaseBinding(SharedKObject global) : global(global), database(NULL)
	{
		/**
		 * @tiapi(method=True,name=Database.open,since=0.4) open a database
		 * @tiarg(for=Database.open,name=name,type=string) database name
		 */
		this->SetMethod("open",&DatabaseBinding::Open);
		/**
		 * @tiapi(method=True,name=Database.execute,since=0.4) open a database
		 * @tiarg(for=Database.execute,name=sql,type=string) sql
		 * @tiresult(for=Database.execute,type=object) returns a Database.ResultSet
		 */
		this->SetMethod("execute",&DatabaseBinding::Execute);
		/**
		 * @tiapi(method=True,name=Database.close,since=0.4) close an open database
		 */
		this->SetMethod("close",&DatabaseBinding::Close);
		/**
		 * @tiapi(method=True,name=Database.remove,since=0.4) remove a database
		 */
		this->SetMethod("remove",&DatabaseBinding::Remove);

		/**
		 * @tiapi(property=True,name=Database.lastInsertRowId) the last id of the inserted row or -1 if not valid
		 */
		SET_INT_PROP("lastInsertRowId",-1);

		/**
		 * @tiapi(property=True,name=Database.rowsAffected) the number of rows affected by the last execute
		 */
		SET_INT_PROP("rowsAffected",0);
	}
	DatabaseBinding::~DatabaseBinding()
	{
		if (database)
		{
			delete database;
			database=NULL;
		}
	}
	std::string DatabaseBinding::GetSecurityOrigin(std::string &appid)
	{
		//this code is loosely based on:
		//http://www.opensource.apple.com/darwinsource/Current/WebCore-5525.18.1/platform/SecurityOrigin.cpp
		std::string origin = "app_"; // protocol which is app
		origin+=appid; // host which is the appid
		origin+="_0"; // port which is always 0
		return origin;
	}
	void DatabaseBinding::Open(const ValueList& args, SharedValue result)
	{
		//FIXME: name can be optional which is "unnamed"
		ArgUtils::VerifyArgsException("open", args, "s?");
		
		if (database)
		{
			delete database;
		}
		std::string appid = AppConfig::Instance()->GetAppID();
		std::string dir = FileUtils::GetApplicationDataDirectory(appid);
		dbname = args.at(0)->ToString();
		origin = GetSecurityOrigin(appid);
		database = new Databases(dir);
		if (!database->Exists(origin,dbname))
		{
			database->Create(origin,dbname);
		}
	}
	void DatabaseBinding::Execute(const ValueList& args, SharedValue result)
	{
		ArgUtils::VerifyArgsException("execute", args, "s,?l");
		if (database == NULL)
		{
			//TODO:
		}
		
		Statement select(database->GetSession());
		try
		{
			select << args.at(0)->ToString();
			
			if (args.size()>1)
			{
				for (size_t c=1;c<args.size();c++)
				{
					SharedValue arg = args.at(c);
					if (arg->IsString())
					{
						std::string s = arg->ToString();
						select , use(s);
					}
					else if (arg->IsInt())
					{
						int i = arg->ToInt();
						select , use(i);
					}
					else if (arg->IsDouble())
					{
						double d = arg->ToDouble();
						select , use(d);
					}
					else if (arg->IsBool())
					{
						bool b = arg->ToBool();
						select , use(b);
					}
					else
					{
						//TODO: throw exception
					}
				}
			}
			Poco::UInt32 count = select.execute();
			SET_INT_PROP("rowsAffected",count);
			if (count > 0)
			{
				// Statement select2(this->session->GetSession());
				// Poco::UInt32 seq;
				// select2 << "SELECT seq FROM sqlite_sequence WHERE name='Databases'", into(seq), now;
				// SET_INT_PROP("lastInsertRowId",seq);
				
				RecordSet rs(select);
				SharedKObject r = new ResultSetBinding(rs);
				result->SetObject(r);
			}
			else
			{
				result->SetObject(new ResultSetBinding());
			}
		}
		catch(Poco::Data::SQLite::InvalidSQLStatementException &se)
		{
			// FIXME
		}
	}
	void DatabaseBinding::Close(const ValueList& args, SharedValue result)
	{
		if (database)
		{
			delete database;
			database = NULL;
		}
	}
	void DatabaseBinding::Remove(const ValueList& args, SharedValue result)
	{
		if (database)
		{
			database->Delete(origin,dbname);
			delete database;
			database = NULL;
		}
	}
}
