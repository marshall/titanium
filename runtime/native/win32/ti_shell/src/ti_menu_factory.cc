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

#include "ti_menu_factory.h"
#include "ti_system_menu.h"

TiMenuFactory::TiMenuFactory()
{
	BindMethod("createSystemMenu", &TiMenuFactory::createSystemMenu);
}

void TiMenuFactory::createSystemMenu(const CppArgumentList& args, CppVariant* result)
{
	if (args.size() >= 3) {
		if (args[0].isString() && args[1].isString() && args[2].isObject()) {
			std::string iconURL = args[0].ToString();
			std::string caption = args[1].ToString();
			NPVariant variant;
			args[2].CopyToNPVariant(&variant);

			NPObject* callback = NPVARIANT_TO_OBJECT(variant);

			TiSystemMenu *menu = new TiSystemMenu(iconURL, caption, callback);

			result->Set(menu->ToNPObject());
		}
	}
}

void TiMenuFactory::createUserMenu(const CppArgumentList& args, CppVariant* result)
{

}