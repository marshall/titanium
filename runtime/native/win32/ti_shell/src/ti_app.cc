/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ti_app.h"
#include "ti_window.h"

/*static*/
bool TiApp::stringToBool (const char * b) {
	std::string str = b;
	std::transform(str.begin(), str.end(), str.begin(), tolower);

	if (str == "yes" || str == "true" || str == "on") {
		return true;
	}
	return false;
}

TiWindow* TiApp::getWindow(std::string& id)
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		if (windows[i]->getId() == id) {
			return windows[i];
		}
	}
	return NULL;
}

TiWindow* TiApp::getMainWindow()
{
	if (windows.size() > 0) {
		return windows[0];
	}
	return NULL;
}

TiApp::TiApp(std::wstring& xmlfile)
{
	xmlDocPtr document = xmlReadFile(WideToUTF8(xmlfile).c_str(), NULL, 0);
	
	if (document != NULL) {
		
		xmlNodePtr root = xmlDocGetRootElement(document);
		xmlNodePtr node = root->children;
		while (node != NULL) {
			if (node->type == XML_ELEMENT_NODE) {
				if (nodeNameEquals(node, "name")) {
					appName = nodeValue(node);
				} else if (nodeNameEquals(node, "description")) {
					description = nodeValue(node);
				} else if (nodeNameEquals(node, "copyright")) {
					copyright = nodeValue(node);
				} else if (nodeNameEquals(node, "homepage")) {
					homepage = nodeValue(node);
				} else if (nodeNameEquals(node, "version")) {
					version = nodeValue(node);
				} else if (nodeNameEquals(node, "window")) {
					this->windows.push_back(new TiWindow(this, (xmlElementPtr)node));
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

	xmlCleanupParser();
}

TiApp::~TiApp()
{
}

std::wstring TiApp::getResourcePath()
{
	std::wstring path;
	PathService::Get(base::DIR_EXE, &path);
	file_util::AppendToPath(&path, L"Resources");

	return path;
}