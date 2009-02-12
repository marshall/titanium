/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "snarl_win32.h"
#include "SnarlInterface.h"

using namespace ti;
using namespace kroll;

namespace ti {
	SnarlWin32::SnarlWin32(SharedBoundObject global) : GrowlBinding(global) {

	}

	SnarlWin32::~SnarlWin32() {
		// TODO Auto-generated destructor stub
	}

	bool SnarlWin32::IsRunning()
	{
		SnarlInterface snarlInterface;

		int major, minor;
		return snarlInterface.snGetVersion(&major, &minor);
	}

	void SnarlWin32::ShowNotification(std::string& title, std::string& description, std::string& iconURL, int notification_delay, SharedBoundMethod callback)
	{
		SnarlInterface::SNARLSTRUCT snarlStruct;
		snarlStruct.cmd = SnarlInterface::SNARL_SHOW;

		std::wstring wtitle(title.begin(), title.end());
		std::wstring wdesc(description.begin(), description.end());

		int len;
		char* buf = SnarlInterface::convertToMultiByte(wtitle, &len);
		strncpy(snarlStruct.title, buf, SnarlInterface::SNARL_STRING_LENGTH - 1);
		snarlStruct.title[len] = 0;
		delete[] buf;

		buf = SnarlInterface::convertToMultiByte(wdesc, &len);
		strncpy(snarlStruct.text, buf, SnarlInterface::SNARL_STRING_LENGTH - 1);
		snarlStruct.text[len] = 0;
		delete[] buf;

		if (iconURL.size() > 0) {

			SharedValue iconPathValue = global->CallNS("App.appURLToPath", Value::NewString(iconURL));
			if (iconPathValue->IsString()) {
				std::string iconPath = iconPathValue->ToString();
				std::wstring wicon(iconPath.begin(), iconPath.end());
				buf = SnarlInterface::convertToMultiByte(wicon, &len);
				strncpy(snarlStruct.icon, buf, SnarlInterface::SNARL_STRING_LENGTH - 1);
				snarlStruct.icon[len] = 0;
				delete[] buf;
			}
		}

		snarlStruct.timeout = notification_delay;

		// TODO : we'll need to create a custom window class/empty window just to handle the callback, sigh

		//snarlStruct.lngData2 = reinterpret_cast<long>(hWndReply);
		SnarlInterface::send(snarlStruct);
		//snarlStruct.id = uReplyMsg;
	}
}
