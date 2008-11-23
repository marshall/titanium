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

#include "ti_window.h"
#include "ti_app_config.h"

TiWindow::TiWindow(xmlElementPtr element) :
	maximizable(true), minimizable(true),
	closeable(true), resizable(true),
	usingChrome(true), usingScrollbars(false),
	fullscreen(false), transparency(1.0),
	width(800), height(600),
	minWidth(0), maxWidth(9000),
	minHeight(0), maxHeight(9000)
{
	xmlNodePtr child = element->children;
	while (child != NULL) {
		if (nodeNameEquals(child, "id")) {
			id = nodeValue(child);
		}
		else if (nodeNameEquals(child, "title")) {
			title = nodeValue(child);
		}
		else if (nodeNameEquals(child, "url")) {
			url = nodeValue(child);
		}
		else if (nodeNameEquals(child, "maximizable")) {
			maximizable = boolValue(child);
		}
		else if (nodeNameEquals(child, "minimizable")) {
			minimizable = boolValue(child);
		}
		else if (nodeNameEquals(child, "closeable")) {
			closeable = boolValue(child);
		}
		else if (nodeNameEquals(child, "resizable")) {
			resizable = boolValue(child);
		}
		else if (nodeNameEquals(child, "fullscreen")) {
			fullscreen = boolValue(child);
		}
		else if (nodeNameEquals(child, "chrome")) {
			usingChrome = boolValue(child);
			const char * scrollbars = (const char *)xmlGetProp(child, (const xmlChar *)"scrollbars");
			if (scrollbars != NULL) {
				usingScrollbars = TiAppConfig::stringToBool(scrollbars);
			}
		}
		else if (nodeNameEquals(child, "transparency")) {
			transparency = (float)atof(nodeValue(child));
		}
		else if (nodeNameEquals(child, "width")) {
			width = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "height")) {
			height = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "min-width")) {
			minWidth = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "max-width")) {
			maxWidth = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "min-height")) {
			minHeight = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "max-height")) {
			maxHeight = atoi(nodeValue(child));
		}
		child = child->next;
	}
}