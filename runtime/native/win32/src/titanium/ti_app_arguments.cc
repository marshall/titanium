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
* limitations under the License..
*/
#include "ti_app_arguments.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/file_util.h"

bool TiAppArguments::isDevMode = false;
std::wstring TiAppArguments::xmlPath = L"";

namespace TiSwitches
{
	const wchar_t xml[] = L"xml";
	const wchar_t devMode[] = L"dev-mode";
}

/*static*/
std::wstring TiAppArguments::defaultXmlPath()
{
	std::wstring path;
	PathService::Get(base::DIR_EXE, &path);
	file_util::AppendToPath(&path, L"Resources");
	file_util::AppendToPath(&path, L"tiapp.xml");
	return path;
}

/*static*/
void TiAppArguments::init(wchar_t *command_line)
{
	// initialize here so we don't cause errors trying to access
	// the path service before the at exit manager has been created
	TiAppArguments::xmlPath = TiAppArguments::defaultXmlPath();

	// chromium's command line parser thinks the first arg is the program name
	// but in win32 it isn't passed-in that I can see...
	std::wstring c = L"titanium ";
	c += command_line;

	CommandLine parsed_command_line(c);

	if (parsed_command_line.HasSwitch(TiSwitches::devMode))
	{
		TiAppArguments::isDevMode = true;
	}

	if (parsed_command_line.HasSwitch(TiSwitches::xml))
	{
		TiAppArguments::xmlPath = parsed_command_line.GetSwitchValue(TiSwitches::xml);
	}
}