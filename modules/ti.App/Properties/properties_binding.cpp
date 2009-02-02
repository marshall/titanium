/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "properties_binding.h"
#include <Poco/StringTokenizer.h>
#include <Poco/File.h>

using namespace kroll;

namespace ti {
	PropertiesBinding::PropertiesBinding(kroll::Host *host) {
		file_path = FileUtils::GetApplicationDirectory();
		file_path += KR_PATH_SEP;
		file_path += "application.properties";

		Poco::File file(file_path);
		if (!file.exists()) {
			file.createFile();
		}

		config = new Poco::Util::PropertyFileConfiguration(file_path);

		SetMethod("getBool", &PropertiesBinding::GetBool);
		SetMethod("getDouble", &PropertiesBinding::GetDouble);
		SetMethod("getInt", &PropertiesBinding::GetInt);
		SetMethod("getString", &PropertiesBinding::GetString);
		SetMethod("getList", &PropertiesBinding::GetList);
		SetMethod("setBool", &PropertiesBinding::SetBool);
		SetMethod("setDouble", &PropertiesBinding::SetDouble);
		SetMethod("setInt", &PropertiesBinding::SetInt);
		SetMethod("setString", &PropertiesBinding::SetString);
		SetMethod("setList", &PropertiesBinding::SetList);
		SetMethod("hasProperty", &PropertiesBinding::HasProperty);
		SetMethod("listProperties", &PropertiesBinding::ListProperties);
	}

	PropertiesBinding::~PropertiesBinding() {
		config->save(file_path);
	}


#define Setter(type) Set ## type
#define Getter(type) Get ## type
#define LSetter(type) set ## type
#define LGetter(type) get ## type
#define IsType(type) Is ## type
#define ToType(type) To ## type
#define PropertyGetter(type)\
	if (args.size() > 0 && args.at(0)->IsString()) { \
		std::string eprefix = "PropertiesBinding::Get: "; \
		try { \
			std::string property = args.at(0)->ToString(); \
			if (args.size() == 1) { \
				result->Setter(type)(config->LGetter(type)(property)); \
				return; \
			} \
			else if (args.size() >= 2 && args.at(1)->IsType(type)()) { \
				result->Setter(type)(config->LGetter(type)(property, args.at(1)->ToType(type)())); \
				return; \
			} \
		} catch(Poco::Exception &e) { \
			throw ValueException::FromString(eprefix + e.displayText()); \
		} \
	}

#define PropertySetter(type)\
	if (args.size() >= 2 && args.at(0)->IsString()) { \
		std::string eprefix = "PropertiesBinding::Set: "; \
		try { \
			std::string property = args.at(0)->ToString(); \
			config->LSetter(type)(property, args.at(1)->ToType(type)()); \
			config->save(file_path); \
		} catch(Poco::Exception &e) { \
			throw ValueException::FromString(eprefix + e.displayText()); \
		} \
	}

	void PropertiesBinding::GetBool(const ValueList& args, SharedValue result)
	{
		PropertyGetter(Bool)
	}

	void PropertiesBinding::GetDouble(const ValueList& args, SharedValue result)
	{
		PropertyGetter(Double)
	}

	void PropertiesBinding::GetInt(const ValueList& args, SharedValue result)
	{
		PropertyGetter(Int)
	}

	void PropertiesBinding::GetString(const ValueList& args, SharedValue result)
	{
		PropertyGetter(String)
	}

	void PropertiesBinding::GetList(const ValueList& args, SharedValue result)
	{
		SharedValue stringValue = Value::Null;
		GetString(args, stringValue);

		if (!stringValue->IsNull()) {
			SharedPtr<StaticBoundList> list = new StaticBoundList();
			std::string string = stringValue->ToString();
			Poco::StringTokenizer t(string, ",", Poco::StringTokenizer::TOK_TRIM);
			for (size_t i = 0; i < t.count(); i++) {
				SharedValue token = Value::NewString(t[i].c_str());
				list->Append(token);
			}

			SharedBoundList list2 = list;
			result->SetList(list2);
		}
	}

	void PropertiesBinding::SetBool(const ValueList& args, SharedValue result)
	{
		PropertySetter(Bool)
	}

	void PropertiesBinding::SetDouble(const ValueList& args, SharedValue result)
	{
		PropertySetter(Double)
	}

	void PropertiesBinding::SetInt(const ValueList& args, SharedValue result)
	{
		PropertySetter(Int)
	}

	void PropertiesBinding::SetString(const ValueList& args, SharedValue result)
	{
		PropertySetter(String)
	}

	void PropertiesBinding::SetList(const ValueList& args, SharedValue result)
	{
		if (args.size() >= 2 && args.at(0)->IsString() && args.at(1)->IsList()) {
			std::string property = args.at(0)->ToString();
			SharedBoundList list = args.at(1)->ToList();

			std::string value;
			for (size_t i = 0; i < list->Size(); i++) {
				value += list->At(i)->ToString();
				if (i < list->Size() - 1) {
					value += ",";
				}
			}
			config->setString(property, value);
			config->save(file_path);
		}
	}

	void PropertiesBinding::HasProperty(const ValueList& args, SharedValue result)
	{
		result->SetBool(false);

		if (args.size() >= 1 && args.at(0)->IsString()) {
			std::string property = args.at(0)->ToString();
			result->SetBool(config->hasProperty(property));
		}
	}

	void PropertiesBinding::ListProperties(const ValueList& args, SharedValue result)
	{
		std::vector<std::string> keys;
		config->keys(keys);

		SharedPtr<StaticBoundList> property_list = new StaticBoundList();
		for (int i = 0; i < keys.size(); i++) {
			std::string property_name = keys.at(i);
			SharedValue name_value = Value::NewString(property_name.c_str());
			property_list->Append(name_value);
		}
		result->SetList(property_list);
	}
}
