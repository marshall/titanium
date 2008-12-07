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

#include "ti_app_config.h"
#include "ti_window_config.h"
#include "googleurl/src/gurl.h"

int TiWindowConfig::DEFAULT_POSITION = -1;

void TiWindowConfig::setDefaults ()
{
	maximizable = minimizable = closeable = resizable = true;
	usingChrome = usingScrollbars = fullscreen = false;
	transparency = 1.0;
	width = 800;
	height = 600;
	x = y = TiWindowConfig::DEFAULT_POSITION;
	minWidth = minHeight = 0;
	maxWidth = maxHeight = 9000;
	url = "app://" + TiAppConfig::instance()->getAppID() + "/index.html";
	title = "Titanium Application";
	visible = true;
}

std::string TiWindowConfig::insertAppID(std::string url)
{
	GURL gurl(url);
	GURL::Replacements replacements;

	if (gurl.SchemeIs("app")) {
		replacements.SetHostStr(TiAppConfig::instance()->getAppID());
		std::string path = "/" + gurl.host();
		if (gurl.has_path() && gurl.path() != "/") {
			path += gurl.path();
		}

		replacements.SetPathStr(path);

		gurl = gurl.ReplaceComponents(replacements);
	}

	return gurl.spec();
}

TiWindowConfig::TiWindowConfig(xmlElementPtr element)
{
	setDefaults();

	xmlNodePtr child = element->children;
	while (child != NULL) {
		if (nodeNameEquals(child, "id")) {
			id = nodeValue(child);
		}
		else if (nodeNameEquals(child, "title")) {
			title = nodeValue(child);
		}
		else if (nodeNameEquals(child, "url")) {
			url = insertAppID(nodeValue(child));
		}
		else if (nodeNameEquals(child, "maximizable")) {
			maximizable = boolValue(child);
		}
		else if (nodeNameEquals(child, "minimizable")) {
			minimizable =	 boolValue(child);
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

std::string TiWindowConfig::toString()
{
	std::ostringstream stream;

	stream << "[TiWindowConfig id=" << id
		<< ", x=" << x << ", y=" << y
		<< ", width=" << width << ", height=" << height
		<< ", url=" << url
		<< "]";

	return stream.str();
}