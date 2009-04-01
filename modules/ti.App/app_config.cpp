/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "app_config.h"
#include "window_config.h"
#include "config_utils.h"
#include "Poco/RegularExpression.h"

using namespace ti;
AppConfig *AppConfig::instance_ = NULL;

AppConfig::AppConfig(std::string& xmlfile)
{
	instance_ = this;
	error = NULL;
	xmlParserCtxtPtr context = xmlNewParserCtxt();

	xmlDocPtr document = xmlCtxtReadFile(context, xmlfile.c_str(), NULL, 0);
	if (document != NULL)
	{
		xmlNodePtr root = xmlDocGetRootElement(document);
		xmlNodePtr node = root->children;
		while (node != NULL)
		{
			if (node->type == XML_ELEMENT_NODE)
			{
				if (nodeNameEquals(node, "name"))
				{
					appName = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "id"))
				{
					appID = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "description"))
				{
					description = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "copyright"))	
				{
					copyright = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "homepage"))
				{
					homepage = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "version"))
				{
					version = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "updatesite"))
				{
					updateSite = ConfigUtils::GetNodeValue(node);
				}
				else if (nodeNameEquals(node, "window"))
				{
					this->windows.push_back(new WindowConfig((void *) node));
				}
				else if (nodeNameEquals(node, "icon"))
				{
					xmlNodePtr child = node->children;
					while (child != NULL)
					{
						if (child->type == XML_ELEMENT_NODE)
						{
							if (nodeNameEquals(child, "image16"))
							{
								icon16 = ConfigUtils::GetNodeValue(child);
							}
							else if (nodeNameEquals(child, "image32"))
							{
								icon32 = ConfigUtils::GetNodeValue(child);
							}
							else if (nodeNameEquals(child, "image48"))
							{
								icon48 = ConfigUtils::GetNodeValue(child);
							}
						}
						child = child->next;
					}
				}
			}
			node = node->next;
		}

		xmlFreeDoc(document);
	}

	if (document == NULL)
	{
		std::string _error;

		if (context->lastError.code != XML_IO_LOAD_ERROR)
		{
			_error += context->lastError.file;
			_error += " [Line ";

			std::ostringstream o;
			o << context->lastError.line;

			_error += o.str();
			_error += "]";
			_error += " ";
		}
		_error += context->lastError.message;

		error = strdup(_error.c_str());
	}

	xmlFreeParserCtxt(context);
	xmlCleanupParser();
}

AppConfig::~AppConfig()
{
}

WindowConfig* AppConfig::GetWindow(std::string& id)
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		if (windows[i]->GetID() == id)
		{
			return windows[i];
		}
	}
	return NULL;
}

WindowConfig* AppConfig::GetWindowByURL(std::string url)
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		std::string urlRegex(windows[i]->GetURLRegex());

		Poco::RegularExpression::Match match;
		Poco::RegularExpression regex(urlRegex);

		regex.match(url, match);

		if(match.length != 0)
		{
			return windows[i];
		}
	}
	return NULL;
}

WindowConfig* AppConfig::GetMainWindow()
{
	if (windows.size() > 0)
		return windows[0];
	else
		return NULL;
}

std::string AppConfig::InsertAppIDIntoURL(std::string url)
{
	std::string appid = this->GetAppID();
	std::transform(appid.begin(), appid.end(), appid.begin(), tolower);

	std::string lcurl = url;
	std::transform(lcurl.begin(), lcurl.end(), lcurl.begin(), tolower);
	if (lcurl.find("app://") == 0 && lcurl.find(appid, 6) == std::string::npos)
	{
		url = std::string("app://") + appid + "/" + (url.c_str() + 6);
	}
	return url;
}
