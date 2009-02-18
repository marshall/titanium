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

#define SET_STRING(name, prop) \
{ \
	SharedValue v = properties->Get(#name); \
	if (v->IsString()) \
	{ \
		std::string value(v->ToString()); \
		this->prop = value; \
	} \
}

#define SET_BOOL(name, prop) \
{ \
	SharedValue v = properties->Get(#name); \
	if (v->IsString()) \
	{ \
		std::string value(v->ToString()); \
		if (value=="yes" || value=="1" || value=="true" || value=="True") \
		{\
			this->prop = true; \
		} \
		else \
		{ \
			this->prop = false; \
		} \
	} \
	else if (v->IsInt()) \
	{ \
		this->prop = v->ToInt() == 1; \
	} \
	else if (v->IsBool()) \
	{ \
		this->prop = v->ToBool(); \
	} \
}

#define SET_INT(name, prop) \
{ \
	SharedValue v = properties->Get(#name); \
	if (v->IsNumber()) \
	{ \
		this->prop = v->ToInt(); \
	} \
}

#define SET_DOUBLE(name, prop) \
{ \
	SharedValue v = properties->Get(#name); \
	if (v->IsNumber()) \
	{ \
		this->prop = v->ToDouble(); \
	} \
}

using namespace ti;

int WindowConfig::DEFAULT_POSITION = -1;
int WindowConfig::window_count = 0;

void WindowConfig::SetDefaults ()
{

	WindowConfig::window_count++;
	std::ostringstream winid;
	winid << "win_" << WindowConfig::window_count;
	this->winid = winid.str();

	this->maximizable = true;
	this->minimizable = true;
	this->closeable = true;
	this->resizable = true;

	this->usingChrome = true;
	this->usingScrollbars = true;
	this->fullscreen = false;
	this->visible = true;
	this->topMost = false;

	this->transparency = 1.0;
	this->width = 800;
	this->height = 600;
	this->x = WindowConfig::DEFAULT_POSITION;
	this->y = WindowConfig::DEFAULT_POSITION;

	this->minWidth = 0;
	this->minHeight = 0;

	this->maxWidth = 9000;
	this->maxHeight = 9000;

	this->url = "app://" + AppConfig::Instance()->GetAppID() + "/index.html";
	this->title = "Titanium Application";
}

void WindowConfig::UseProperties(SharedBoundObject properties)
{
	this->SetDefaults();

	SET_STRING(id, winid)
	SET_STRING(url, url);
	SET_STRING(urlRegex, urlRegex);
	SET_STRING(title, title);
	SET_INT(x, x);
	SET_INT(y, y);
	SET_INT(width, width);
	SET_INT(minWidth, minWidth);
	SET_INT(maxWidth, maxWidth);
	SET_INT(height, height);
	SET_INT(minHeight, minHeight);
	SET_INT(maxHeight, maxHeight);
	SET_BOOL(visible, visible);
	SET_BOOL(maximizable, maximizable);
	SET_BOOL(minimizable, minimizable);
	SET_BOOL(closeable, closeable);
	SET_BOOL(resizable, resizable);
	SET_BOOL(fullscreen, fullscreen);
	SET_BOOL(usingChrome, usingChrome);
	SET_BOOL(usingScrollbars, usingScrollbars);
	SET_BOOL(topMost, topMost);
	SET_DOUBLE(transparency, transparency);
}

WindowConfig::WindowConfig(WindowConfig *config, std::string& url)
{
	this->SetDefaults();
	this->url = url;

	if (config == NULL) // Just use defaults if not found
		return;

	this->title = config->GetTitle();
	this->x = config->GetX();
	this->y = config->GetY();
	this->width = config->GetWidth();
	this->minWidth = config->GetMinWidth();
	this->maxWidth = config->GetMaxWidth();
	this->height = config->GetHeight();
	this->minHeight = config->GetMinHeight();
	this->maxHeight = config->GetMaxHeight();
	this->visible = config->IsVisible();
	this->maximizable = config->IsMaximizable();
	this->minimizable = config->IsMinimizable();
	this->resizable = config->IsResizable();
	this->fullscreen = config->IsFullScreen();
	this->usingChrome = config->IsUsingChrome();
	this->usingScrollbars = config->IsUsingScrollbars();
	this->topMost = config->IsTopMost();
	this->transparency = config->GetTransparency();

}
WindowConfig::WindowConfig(void* data)
{
	xmlElementPtr element = (xmlElementPtr) data;
	SetDefaults();

	xmlNodePtr child = element->children;
	while (child != NULL) {
		if (nodeNameEquals(child, "id")) {
			winid = nodeValue(child);
		}
		else if (nodeNameEquals(child, "title")) {
			title = nodeValue(child);
		}
		else if (nodeNameEquals(child, "url")) {
			url = nodeValue(child);
		}
		else if (nodeNameEquals(child, "urlRegex")) {
			urlRegex = nodeValue(child);
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
				usingScrollbars = AppConfig::StringToBool(scrollbars);
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

std::string WindowConfig::ToString()
{
	std::ostringstream stream;

	stream << "[WindowConfig id=" << winid
		<< ", x=" << x << ", y=" << y
		<< ", width=" << width << ", height=" << height
		<< ", url=" << url
		<< "]";

	return stream.str();
}
