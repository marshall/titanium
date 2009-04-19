/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "database_binding.h"
#include "resultset_binding.h"

namespace ti
{
	DatabaseBinding::DatabaseBinding(Host *host) : host(host), database(NULL), session(NULL)
	{
		/**
		 * @tiapi(method=True,name=Database.DB.execute,since=0.4) open a database
		 * @tiarg(for=Database.execute,name=sql,type=string) sql
		 * @tiresult(for=Database.execute,type=object) returns a Database.ResultSet
		 */
		this->SetMethod("execute",&DatabaseBinding::Execute);
		/**
		 * @tiapi(method=True,name=Database.DB.close,since=0.4) close an open database
		 */
		this->SetMethod("close",&DatabaseBinding::Close);
		/**
		 * @tiapi(method=True,name=Database.DB.remove,since=0.4) remove a database
		 */
		this->SetMethod("remove",&DatabaseBinding::Remove);

		//
		// we don't currently support the lastInsertRowId
		// since poco doesn't (yet) support the method:
		// sqlite3_last_insert_rowid
		// which returns the value from sqlite.  we need
		// to patch poco somehow to expose this..
		//
		SET_INT_PROP("lastInsertRowId",0);
		
		/**
		 * @tiapi(property=True,name=Database.DB.rowsAffected) the number of rows affected by the last execute
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
		if (session)
		{
			delete session;
			session = NULL;
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
			database = NULL;
		}
		if (session)
		{
			delete session;
			session = NULL;
		}
		std::string appid = host->GetApplicationID();
		std::string dbdir = FileUtils::GetApplicationDataDirectory(appid);
		dbname = args.at(0)->ToString();
		origin = GetSecurityOrigin(appid);

		static Logger &logger = Logger::Get("Database");
		logger.Debug("appid=%s,dir=%s,dbname=%s,origin=%s",appid.c_str(),dbdir.c_str(),dbname.c_str(),origin.c_str());

		database = new Databases(dbdir);
		std::string path;
		if (!database->Exists(origin,dbname))
		{
			path = database->Create(origin,dbname);
		}
		else
		{
			path = database->Path(origin,dbname);
		}
		session = new DBSession(path);
	}
	void DatabaseBinding::Execute(const ValueList& args, SharedValue result)
	{
		ArgUtils::VerifyArgsException("execute", args, "s,?l");
		
		if (database == NULL)
		{
			throw ValueException::FromString("no database opened");
		}
		std::string sql = args.at(0)->ToString();

		static Logger &logger = Logger::Get("Database");
		logger.Debug("Execute called with %s",sql.c_str());
		
		Statement select(session->GetSession());
		
		logger.Debug("After creating statement");
		
		
		try
		{
			select << sql;
			
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
						char msg[255];
						sprintf("unknown supported type: %s for argument %d",arg->ToTypeString(),c);
						throw ValueException::FromString(msg);
					}
				}
			}
			Poco::UInt32 count = select.execute();
			logger.Debug("sql returned: %d rows for result",count);
			SET_INT_PROP("rowsAffected",count);

			if (count > 0)
			{
				RecordSet rs(select);
				SharedKObject r = new ResultSetBinding(rs);
				result->SetObject(r);
			}
			else
			{
				SharedKObject r = new ResultSetBinding();
				result->SetObject(r);
			}
		}
		catch(Poco::Data::DataException &e)
		{
			logger.Error("Exception executing: %s, Error was: %s",sql.c_str(),e.what());
			throw ValueException::FromString(e.what());
		}
	}
	void DatabaseBinding::Close(const ValueList& args, SharedValue result)
	{
		static Logger &logger = Logger::Get("Database");
		logger.Debug("Close database: %s",dbname.c_str());
		if (session)
		{
			delete session;
			session = NULL;
		}
		if (database)
		{
			delete database;
			database = NULL;
		}
	}
	void DatabaseBinding::Remove(const ValueList& args, SharedValue result)
	{
		static Logger &logger = Logger::Get("Database");
		logger.Debug("Remove database: %s",dbname.c_str());
		if (session)
		{
			delete session;
			session = NULL;
		}
		if (database)
		{
			database->Delete(origin,dbname);
			delete database;
			database = NULL;
		}
	}
}
