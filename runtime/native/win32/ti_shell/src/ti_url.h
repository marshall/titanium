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

#ifndef TI_URL_H_
#define TI_URL_H_

#include <string>
#include "base/path_service.h"
#include "base/file_util.h"
#include "base/string_util.h"
#include "googleurl/src/gurl.h"
#include "ti_app_config.h"

class TiURL
{
public:
	static std::string absolutePathForURL(TiAppConfig *appConfig, std::string url);
};
#endif