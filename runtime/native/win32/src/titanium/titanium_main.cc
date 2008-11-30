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

#include "base/path_service.h"
#include "base/process_util.h"
#include "base/file_util.h"
#include "base/at_exit.h"
#include "base/win_util.h"
//#include "base/message_loop.h"
#include "sandbox/src/dep.h"

#include "titanium_dll_main.h"

int APIENTRY wWinMain (HINSTANCE instance, HINSTANCE prev_instance,
					   wchar_t* command_line, int n_command_line)

//int main (int argc, char *argv[])
{
	base::EnableTerminationOnHeapCorruption();

	// The exit manager is in charge of calling the dtors of singletons.
	base::AtExitManager exit_manager;
	
	win_util::WinVersion win_version = win_util::GetWinVersion();
	if (win_version == win_util::WINVERSION_XP ||
		win_version == win_util::WINVERSION_SERVER_2003) {
			// On Vista, this is unnecessary since it is controlled through the
			// /NXCOMPAT linker flag.
			// Enforces strong DEP support.
			sandbox::SetCurrentProcessDEP(sandbox::DEP_ENABLED);
	}

	//if (dll_handle != NULL) {
		// default behavior
		std::wstring path;

		PathService::Get(base::DIR_EXE, &path);
		file_util::AppendToPath(&path, L"Resources");
		file_util::AppendToPath(&path, L"tiapp.xml");

		//RunFunc run = reinterpret_cast<RunFunc>(
		//	::GetProcAddress(dll_handle, "runTitaniumApp"));
		//if (NULL != run) {
		return runTitaniumApp(path);
		//}
	//}

	return -1;
}