/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "app_config.h"

using namespace ti;

AppConfig *AppConfig::instance_ = NULL;

/*static*/
bool AppConfig::StringToBool (const char * b) {
	std::string str(b);
	std::transform(str.begin(), str.end(), str.begin(), tolower);

	if (str == "yes" || str == "true" || str == "on") {
		return true;
	}
	return false;
}

WindowConfig* AppConfig::GetWindow(std::string& id)
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		if (windows[i]->GetID() == id) {
			return windows[i];
		}
	}
	return NULL;
}

WindowConfig* AppConfig::GetMainWindow()
{
	if (windows.size() > 0) {
		return windows[0];
	}
	return NULL;
}

AppConfig::AppConfig(std::string& xmlfile)
{
	instance_ = this;
	error = NULL;
	xmlParserCtxtPtr context = xmlNewParserCtxt();
	
	xmlDocPtr document = xmlCtxtReadFile(context, xmlfile.c_str(), NULL, 0);
	if (document != NULL) {

		xmlNodePtr root = xmlDocGetRootElement(document);
		xmlNodePtr node = root->children;
		while (node != NULL) {
			if (node->type == XML_ELEMENT_NODE) {
				if (nodeNameEquals(node, "name")) {
					appName = nodeValue(node);
				} else if (nodeNameEquals(node, "id")) {
					appID = nodeValue(node);
				} else if (nodeNameEquals(node, "description")) {
					description = nodeValue(node);
				} else if (nodeNameEquals(node, "copyright")) {
					copyright = nodeValue(node);
				} else if (nodeNameEquals(node, "homepage")) {
					homepage = nodeValue(node);
				} else if (nodeNameEquals(node, "version")) {
					version = nodeValue(node);
				} else if (nodeNameEquals(node, "updatesite")) {
					updateSite = nodeValue(node);
				} else if (nodeNameEquals(node, "window")) {
					this->windows.push_back(new WindowConfig((void *) node));
				} else if (nodeNameEquals(node, "icon")) {
					xmlNodePtr child = node->children;
					while (child != NULL) {
						if (child->type == XML_ELEMENT_NODE) {
							if (nodeNameEquals(child, "image16")) {
								icon16 = nodeValue(child);
							} else if (nodeNameEquals(child, "image32")) {
								icon32 = nodeValue(child);
							} else if (nodeNameEquals(child, "image48")) {
								icon48 = nodeValue(child);
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

	if (document == NULL) {
		std::string _error;

		if (context->lastError.code != XML_IO_LOAD_ERROR) {

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
