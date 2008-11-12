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
#import <msxml4.dll>

using namespace MSXML2;

class TiApp 
{
private:
	std::string appName, startPath, title;
	int width, height;

	void readElement(IXMLDOMElementPtr element);

public:
	TiApp(std::wstring& xmlfile);
	~TiApp();

	std::string& getStartPath() { return startPath; }
	std::string& getAppName() { return appName; }
	std::string& getTitle() { return title; }
	int getWidth() { return width; }
	int getHeight() { return height; }
};

#endif