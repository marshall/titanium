/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <kroll/kroll.h>
#include "databases.h"


//FIXME: add SessionPool

namespace ti
{
	Databases::Databases(std::string datadir) : datadir(datadir)
	{
		std::string dbpath = FileUtils::Join(datadir.c_str(),"Databases.db",NULL);
		
		session = new DBSession(dbpath);
		
		// if it doesn't exit, create it
		if (!FileUtils::IsFile(dbpath))
		{
			Statement select(this->session->GetSession());
			select << "CREATE TABLE Origins (origin TEXT UNIQUE ON CONFLICT REPLACE, quota INTEGER NOT NULL ON CONFLICT FAIL)";
			select.execute();
			
			Statement select2(this->session->GetSession());
			select2 << "CREATE TABLE Databases (guid INTEGER PRIMARY KEY AUTOINCREMENT, origin TEXT, name TEXT, displayName TEXT, estimatedSize INTEGER, path TEXT)";
			select2.execute();
		}
	}


	Databases::~Databases()
	{
		if (session)
		{
			delete session;
		}
	}

	std::string Databases::Create(std::string origin, std::string name)
	{
		if (Exists(origin, name))
		{
			return Path(origin,name);
		}
		Statement select(this->session->GetSession());
		Poco::UInt32 seq;
		select << "SELECT seq FROM sqlite_sequence WHERE name='Databases'", into(seq);
		select.execute();
		
		++seq;
		char filename[32];
		sprintf(filename,"%016llx.db",(long long)seq);

		Statement select2(this->session->GetSession());
		select2 << "INSERT INTO Databases (origin, name, path) VALUES (:origin,:name,:path)", use(origin), use(name), use(filename);
		select2.execute();
		
		
		// create the DB file
		std::string fullpath = FileUtils::Join(datadir.c_str(),origin.c_str(),filename,NULL);
		DBSession s(fullpath);
		
		// create the metadata table for WebKit
		Statement select3(s.GetSession());
		select3 << "CREATE TABLE __WebKitDatabaseInfoTable__ (key TEXT NOT NULL ON CONFLICT FAIL UNIQUE ON CONFLICT REPLACE,value TEXT NOT NULL ON CONFLICT FAIL)";
		select3.execute();
		
		Statement select4(s.GetSession());
		select4 << "insert into __WebKitDatabaseInfoTable__ values ('WebKitDatabaseVersionKey','1.0')";
		select4.execute();
		
		return filename;
	}

	void Databases::Delete (std::string origin, std::string name)
	{
		if (Exists(origin, name))
		{
			std::string path = Path(origin,name);
			
			Statement select(this->session->GetSession());
			select << "DELETE FROM Databases WHERE origin=:origin AND name=:name", use(origin), use(name);
			select.execute();
			
			//FIXME: remove file
		}
	}

	std::string Databases::Path(std::string origin, std::string name)
	{
		Statement select(this->session->GetSession());
		try
		{
			std::string path;
			select << "SELECT path FROM Databases WHERE origin=:origin AND name=:name", use(origin), use(name), into(path);
			Poco::UInt32 count = select.execute();
			if (count > 0)
			{
				return path;
			}
		}
		catch(Poco::Data::SQLite::InvalidSQLStatementException &se)
		{
			// NO DB
		}
		return "";
	}

	bool Databases::Exists(std::string origin, std::string name)
	{
		Statement select(this->session->GetSession());
		try
		{
			select << "SELECT guid FROM Databases WHERE origin=:origin AND name=:name", use(origin), use(name);
			Poco::UInt32 count = select.execute();
			return count > 0;
		}
		catch(Poco::Data::SQLite::InvalidSQLStatementException &se)
		{
			// NO DB
			return false;
		}
	}
}