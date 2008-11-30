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
#ifndef TI_WINDOW_CONFIG_H_
#define TI_WINDOW_CONFIG_H_

#include <string>

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"

#include "ti_app_config.h"

class TiWindowConfig
{
private:
	std::string id, url, title;
	int x, y, width, height, minWidth, minHeight, maxWidth, maxHeight;
	float transparency;

	bool visible, maximizable, minimizable, closeable,
		resizable, fullscreen, usingChrome, usingScrollbars;

	void setDefaults();
public:
	static int DEFAULT_POSITION;

	TiWindowConfig() { setDefaults(); }
	TiWindowConfig(xmlElementPtr window);
	
	std::string toString();

	// window accessors
	std::string& getURL() { return url; }
	void setURL(std::string& url_) { url = url_; }
	std::string& getTitle() { return title; }
	void setTitle(std::string& title_) { title = title_; }
	std::string& getId() { return id; }

	int getX() { return x; }
	void setX(int x_) { x = x_; }
	int getY() { return y; }
	void setY(int y_) { y = y_; }
	int getWidth() { return width; }
	void setWidth(int width_) { width = width_; }
	int getHeight() { return height; }
	void setHeight(int height_) { height = height_; }
	int getMinWidth() { return minWidth; }
	void setMinWidth(int minWidth_) { minWidth = minWidth_; }
	int getMinHeight() { return minHeight; }
	void setMinHeight(int minHeight_) { minHeight = minHeight_; }
	int getMaxWidth() { return maxWidth; }
	void setMaxWidth(int maxWidth_) { maxWidth = maxWidth_; }
	int getMaxHeight() { return maxHeight; }
	void setMaxHeight(int maxHeight_) { maxHeight = maxHeight_; }

	float getTransparency() { return transparency; }
	void setTransparency(float transparency_) { transparency = transparency_; }
	bool isVisible() { return visible; }
	void setVisible(bool visible_) { visible = visible_; }
	bool isMaximizable() { return maximizable; }
	void setMaximizable(bool maximizable_) { maximizable = maximizable_; }
	bool isMinimizable() { return minimizable; }
	void setMinimizable(bool minimizable_) { minimizable = minimizable_; }
	bool isCloseable() { return closeable; }
	void setCloseable(bool closeable_) { closeable = closeable_; }
	bool isResizable() { return resizable; }
	void setResizable(bool resizable_) { resizable = resizable_; }
	bool isFullscreen() { return fullscreen; }
	void setFullscreen(bool fullscreen_) { fullscreen = fullscreen_; }
	bool isUsingChrome() { return usingChrome; }
	void setUsingChrome(bool usingChrome_) { usingChrome = usingChrome_; }
	bool isUsingScrollbars() { return usingScrollbars; }
	void setUsingScrollbars(bool usingScrollbars_) { usingScrollbars = usingScrollbars_; }
};

#endif