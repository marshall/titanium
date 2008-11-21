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
#ifndef TI_WINDOW_H_
#define TI_WINDOW_H_

#include <string>

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"

class TiWindow
{
private:
	std::string id, url, title;
	int width, height, minWidth, minHeight, maxWidth, maxHeight;
	float transparency;

	bool maximizable, minimizable, closeable,
		resizable, fullscreen, usingChrome, usingScrollbars;
public:
	TiWindow(xmlElementPtr window);
	
	// window accessors
	std::string& getURL() { return url; }
	std::string& getTitle() { return title; }
	std::string& getId() { return id; }

	int getWidth() { return width; }
	int getHeight() { return height; }
	int getMinWidth() { return minWidth; }
	int getMinHeight() { return minHeight; }
	int getMaxWidth() { return maxWidth; }
	int getMaxHeight() { return maxHeight; }

	float getTransparency() { return transparency; }
	bool isMaximizable() { return maximizable; }
	bool isMinimizable() { return minimizable; }
	bool isCloseable() { return closeable; }
	bool isResizable() { return resizable; }
	bool isFullscreen() { return fullscreen; }
	bool isUsingChrome() { return usingChrome; }
	bool isUsingScrollbars() { return usingScrollbars; }

};

#endif