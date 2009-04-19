/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _DATABASE_BINDING_H_
#define _DATABASE_BINDING_H_

#include <kroll/kroll.h>
#include "databases.h"
#include "db_session.h"

namespace ti
{
	class DatabaseBinding : public StaticBoundObject
	{
	public:
		DatabaseBinding();
	protected:
		virtual ~DatabaseBinding();
	private:
		Databases *database;
		DBSession *session;
		std::string dbname;
		std::string origin;
		
		std::string GetSecurityOrigin(std::string &appid);

		DECLAREBOUNDMETHOD(Open);
		DECLAREBOUNDMETHOD(Execute);
		DECLAREBOUNDMETHOD(Close);
		DECLAREBOUNDMETHOD(Remove);
		
		friend class DatabaseModule;
	};
}

#endif
