#ifndef TI_APP_H_
#define TI_APP_H_
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
#include <string>
#include <algorithm>

#import <msxml6.dll>

using namespace MSXML2;

class TiApp 
{
private:
	std::string appName, description, copyright, homepage, version;
	
	// window properties
	std::string startPath, title;
	int width, height;
	float transparency;
	bool maximizable, minimizable, closeable,
		resizable, usingChrome, usingScrollbars;

	// icon properties
	std::string icon16, icon32, icon48;

	void readElement(IXMLDOMElementPtr element);
	void readWindowElement(IXMLDOMElementPtr element);
	void readIconElement(IXMLDOMElementPtr element);

public:
	TiApp(std::wstring& xmlfile);
	~TiApp();

	std::string& getAppName() { return appName; }
	std::string& getDescription() { return description; }
	std::string& getCopyright() { return copyright; }
	std::string& getHomepage() { return homepage; }
	std::string& getVersion() { return version; }

	// window accessors
	std::string& getStartPath() { return startPath; }
	std::string& getTitle() { return title; }
	
	int getWidth() { return width; }
	int getHeight() { return height; }

	float getTransparency() { return transparency; }
	bool isMaximizable() { return maximizable; }
	bool isMinimizable() { return minimizable; }
	bool isCloseable() { return closeable; }
	bool isResizable() { return resizable; }
	bool isUsingChrome() { return usingChrome; }
	bool isUsingScrollbars() { return usingScrollbars; }

	//icon accessors
	std::string& getIcon16() { return icon16; }
	std::string& getIcon32() { return icon32; }
	std::string& getIcon48() { return icon48; }
};

#endif