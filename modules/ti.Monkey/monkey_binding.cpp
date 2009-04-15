/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "monkey_binding.h"
#include <sstream>
#include <functional>
#include <Poco/Path.h>
#include <Poco/FileStream.h>


namespace ti
{
	MonkeyBinding::MonkeyBinding(Host *host, SharedKObject global) : global(global), registration(0)
	{
		const std::string home = host->GetApplicationHomePath();
		
		// check to make sure we have a user scripts directory
		std::string dir = FileUtils::Join(home.c_str(),"Resources","userscripts",NULL);

		PRINTD("USER SCRIPT DIR = " << dir);

		if (FileUtils::IsDirectory(dir))
		{
			std::vector<std::string> files;
			FileUtils::ListDir(dir,files);
			if (files.size()>0)
			{
				std::vector<std::string>::iterator iter = files.begin();
				while(iter!=files.end())
				{
					std::string file = (*iter++);
					std::string fn = FileUtils::Join(dir.c_str(),file.c_str(),NULL);
					Poco::Path path(fn);
					if (path.getExtension() == "js")
					{
						Poco::FileInputStream f(fn);
						std::string line;
						std::ostringstream str;
						bool found = false, start = false;
						VectorOfPatterns includes;
						VectorOfPatterns excludes;
						while (!f.eof())
						{
							std::getline(f, line);
							if (line.find("// ==UserScript==") == 0)
							{
								found = true;
							}
							else if (found && !start)
							{
								std::string::size_type i = line.find("// @include");
								if (i == 0)
								{
									std::string url = FileUtils::Trim(line.substr(i+12).c_str());
									includes.push_back(PatternMatcher(url));
									continue;
								}
								i = line.find("// @exclude");
								if (i == 0)
								{
									std::string url = FileUtils::Trim(line.substr(i+12).c_str());
									excludes.push_back(PatternMatcher(url));
								}
								else if (line.find("// ==/UserScript==") == 0)
								{
									start = true;
									str << "(function(){\n";
								}
								//TODO: @require
							}
							else if (start)
							{
								str << line << "\n";
							}
						}
						if (found && start)
						{
							str << "\n})();";
							try
							{
								std::pair<VectorOfPatterns,VectorOfPatterns> r(includes,excludes);
								scripts.push_back(std::pair<std::pair< VectorOfPatterns,VectorOfPatterns >,std::string>(r,str.str()));
							}
							catch(Poco::RegularExpressionException &e)
							{
								std::cerr << "Exception loading user script: " << fn << " Exception: " << e.what() << std::endl;
							}
						}
					}
				}
				
				if (scripts.size()>0)
				{
					this->SetMethod("callback",&MonkeyBinding::Callback);
					SharedValue v = global->CallNS("API.register",Value::NewString("ti.UI.window.page.load"),this->Get("callback"));
					this->registration = v->ToInt();
				}
			}
		}
	}
	MonkeyBinding::~MonkeyBinding()
	{
		if (registration)
		{
			global->CallNS("API.unregister",Value::NewInt(this->registration));
		}
	}
	bool MonkeyBinding::Matches(VectorOfPatterns patterns, std::string &subject)
	{
		if (patterns.size()==0) return false;
		VectorOfPatterns::iterator iter = patterns.begin();
		while(iter!=patterns.end())
		{
			PatternMatcher m = (*iter++);
			if (m(subject.c_str()))
			{
				return true;
			}
		}
		return false;
	}
	void MonkeyBinding::Callback(const ValueList &args, SharedValue result)
	{
		SharedKObject event = args.at(1)->ToObject();
		std::string url_value = event->Get("url")->ToString();
		
		std::vector< std::pair< std::pair< VectorOfPatterns,VectorOfPatterns >,std::string> >::iterator iter = scripts.begin();
		while(iter!=scripts.end())
		{
			std::pair< std::pair< VectorOfPatterns,VectorOfPatterns >, std::string> e = (*iter++);
			VectorOfPatterns include = e.first.first;
			VectorOfPatterns exclude = e.first.second;

			if (Matches(exclude,url_value))
			{
				continue;
			}
			if (Matches(include,url_value))
			{
				// I got a castle in brooklyn, that's where i dwell 
				try
				{
					SharedKMethod eval = event->Get("scope")->ToObject()->Get("window")->ToObject()->Get("eval")->ToMethod();
#ifdef DEBUG
					std::cout << ">>> loading user script for " << url_value << std::endl;
#endif
					eval->Call(Value::NewString(e.second));
				}
				catch (ValueException &ex)
				{
					SharedValue v = ex.GetValue();
					if (v->IsString())
					{
						std::cerr << "Exception generated evaluating user script for " << url_value << ". Exception: " << v->ToString() <<std::endl;
						continue;
					}
					else if (v->IsObject())
					{
						SharedKObject bo = v->ToObject();
						SharedValue tm = bo->Get("toString");
						if (tm->IsMethod())
						{
							std::cerr << "Exception generated evaluating user script for " << url_value << ". Exception: " << tm->ToMethod()->Call()->ToString() <<std::endl;
							continue;
						}
					}
					std::cerr << "Exception generated evaluating user script for " << url_value << ". Unknown Exception: " << v->ToTypeString() <<std::endl;
				}
				catch (std::exception &ex)
				{
					std::cerr << "Exception generated evaluating user script for " << url_value << ". Exception: " << ex.what()<<std::endl;
				}
			}
		}
	}
}
