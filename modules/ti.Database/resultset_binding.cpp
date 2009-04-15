/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "resultset_binding.h"
#include <Poco/Data/MetaColumn.h>
#include <Poco/DynamicAny.h>

namespace ti
{
	ResultSetBinding::ResultSetBinding() : eof(true)
	{
		// no results result set
		Bind();
	}
	ResultSetBinding::ResultSetBinding(Poco::Data::RecordSet &r) : eof(false)
	{
		rs = new Poco::Data::RecordSet(r);
		Bind();
	}
	void ResultSetBinding::Bind()
	{
		/**
		 * @tiapi(method=True,name=Database.ResultSet.isValidRow,since=0.4) Returns true if you can call data extraction methods
		 * @tiresult(for=Database.ResultSet.isValidRow,type=boolean) true if valid
		 */
		this->SetMethod("isValidRow",&ResultSetBinding::IsValidRow);

		/**
		 * @tiapi(method=True,name=Database.ResultSet.isValidRow,since=0.4) Advances to the next row of the results.
		 */
		this->SetMethod("next",&ResultSetBinding::Next);

		/**
		 * @tiapi(method=True,name=Database.ResultSet.close,since=0.4) Releases the state associated with this result set
		 */
		this->SetMethod("close",&ResultSetBinding::Close);

		/**
		 * @tiapi(method=True,name=Database.ResultSet.fieldCount,since=0.4) Returns the number of fields in this result set.
		 * @tiresult(for=Database.ResultSet.fieldCount,type=integer) count
		 */
		this->SetMethod("fieldCount",&ResultSetBinding::FieldCount);

		/**
		 * @tiapi(method=True,name=Database.ResultSet.fieldName,since=0.4) Returns the name of the specified field in the current result set. This name is derived from the SQL statement which was executed.
		 * @tiarg(for=Database.ResultSet.fieldName,type=integer,name=fieldIndex) the zero-based index of the desired field
		 * @tiresult(for=Database.ResultSet.fieldName,type=string) result
		 */
		this->SetMethod("fieldName",&ResultSetBinding::FieldName);

		/**
		 * @tiapi(method=True,name=Database.ResultSet.field,since=0.4) Returns the contents of the specified field in the current row.
		 * @tiarg(for=Database.ResultSet.field,type=integer,name=fieldIndex) the zero-based index of the desired field
		 * @tiresult(for=Database.ResultSet.field,type=object) result
		 */
		this->SetMethod("field",&ResultSetBinding::Field);

		/**
		 * @tiapi(method=True,name=Database.ResultSet.fieldByName,since=0.4) Returns the contents of the specified field in the current row.
		 * @tiarg(for=Database.ResultSet.fieldByName,type=string,name=name) the name of the desired field
		 * @tiresult(for=Database.ResultSet.fieldByName,type=object) result
		 */
		this->SetMethod("fieldByName",&ResultSetBinding::FieldByName);
	}
	ResultSetBinding::~ResultSetBinding()
	{
	}
	void ResultSetBinding::IsValidRow(const ValueList& args, SharedValue result)
	{
		if (rs.isNull())
		{
			result->SetBool(false);
		}
		else
		{
			result->SetBool(!eof);
		}
	}
	void ResultSetBinding::Next(const ValueList& args, SharedValue result)
	{
		if (!rs.isNull() && !eof)
		{
			eof = (rs->moveNext() == false);
			result->SetBool(!eof);
		}
		else
		{
			result->SetBool(false);
		}
	}
	void ResultSetBinding::Close(const ValueList& args, SharedValue result)
	{
		if (!rs.isNull())
		{
			rs = NULL;
		}
	}
	void ResultSetBinding::FieldCount(const ValueList& args, SharedValue result)
	{
		if (rs.isNull())
		{
			result->SetInt(0);
		}
		else
		{
			result->SetInt(rs->rowCount());
		}
	}
	void ResultSetBinding::FieldName(const ValueList& args, SharedValue result)
	{
		if (rs.isNull())
		{
			result->SetNull();
		}
		else
		{
			ArgUtils::VerifyArgsException("fieldName", args, "i");
			const std::string &str = rs->columnName(args.at(0)->ToInt());
			result->SetString(str);
		}
	}
	void ResultSetBinding::Field(const ValueList& args, SharedValue result)
	{
		if (rs.isNull())
		{
			result->SetNull();
		}
		else
		{
			ArgUtils::VerifyArgsException("field", args, "i");
			TransformValue(args.at(0)->ToInt(),result);
		}
	}
	void ResultSetBinding::FieldByName(const ValueList& args, SharedValue result)
	{
		result->SetNull();
		if (!rs.isNull())
		{
			ArgUtils::VerifyArgsException("fieldByName", args, "s");
			std::string name = args.at(0)->ToString();
			size_t count = rs->columnCount();
			for (size_t i = 0; i<count; i++)
			{
				const std::string &str = rs->columnName(i);
				if (str == name)
				{
					TransformValue(i,result);
					break;
				}
			}
		}
	}
	void ResultSetBinding::TransformValue(size_t index, SharedValue result)
	{
		MetaColumn::ColumnDataType type = rs->columnType(index);
		Poco::DynamicAny value = rs->value(index);
		
		if (value.isEmpty())
		{
			result->SetNull();
		}
		else if (type == MetaColumn::FDT_STRING)
		{
			std::string str;
			value.convert(str);
			result->SetString(str);
		}
		else if (type == MetaColumn::FDT_BOOL)
		{
			bool v = false;
			value.convert(v);
			result->SetBool(v);
		}
		else if (type == MetaColumn::FDT_FLOAT || type == MetaColumn::FDT_DOUBLE)
		{
			float f = 0;
			value.convert(f);
			result->SetDouble(f);
		}
		else if (type == MetaColumn::FDT_BLOB || type == MetaColumn::FDT_UNKNOWN)
		{
			std::string str;
			value.convert(str);
			result->SetString(str);
		}
		else
		{
			// the rest of these are ints:
			// FDT_INT8,
			// FDT_UINT8,
			// FDT_INT16,
			// FDT_UINT16,
			// FDT_INT32,
			// FDT_UINT32,
			// FDT_INT64,
			// FDT_UINT64,
			int i;
			value.convert(i);
			result->SetInt(i);
		}
	}
}
