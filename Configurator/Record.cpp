#include "stdafx.h"

#include "Record.h"

using namespace std;

namespace Config
{
	Record::Record()
	{
		Init(L"", L"", L"", L"");
	}
		
	Record::Record( wstring comments, wstring section,  wstring key, wstring default )
	{
		Init(comments, section, key, default);
	}

	void Record::Init( wstring comments, wstring section, wstring key, wstring default )
	{
		this->comments = comments;
		this->section = section;
		this->key = key;
		this->default = default;
		SetValue(this->default);
	}

	std::wstring Record::GetValue() const
	{
		return this->value;
	}

	std::wstring Record::GetDefault() const
	{
		return default;
	}

	bool Record::SetValue( wstring value )
	{
		this->value = value;
		return true;
	}

	DwordRecord::DwordRecord(wstring comments, wstring section,  wstring key, wstring default):
		Record(comments, section, key, default)
	{
		SetValue(default);
	};

	bool DwordRecord::SetValue( wstring value )
	{
		this->dwordValue = stoul(value);
		return true;
	}

	DWORD DwordRecord::GetDwordValue()
	{
		return dwordValue; 
	}

	bool DwordRecord::SetValueInInterval( wstring value, DWORD left, DWORD right )
	{
		wstring temp = value;
		if (SetValue(value))
			if (left <= dwordValue && dwordValue <= right)
				return true;
		value = temp;
		return false;
	}

	bool DwordRecord::SetValueInArray( wstring value, const vector<DWORD> & vector)
	{
		wstring temp = value;
		if (SetValue(value))
			for (UINT i = 0; i < vector.size(); ++i)
				if (vector[i] == dwordValue)
					return true;
		value = temp;
		return false;
	}

	ByteRecord::ByteRecord(wstring comments, wstring section, wstring key, wstring default):
		Record(comments, section, key, default)
	{
		SetValue(default);
	};

	bool ByteRecord::SetValue( wstring value )
	{
		int byteVal = stoi(value);
		if (byteVal > 0 && byteVal < 256)
			this->byteValue = byteVal;
		else
		{
			byteVal = stoi(default);
			if (byteVal > 0 && byteVal < 256)
			{
				this->byteValue = byteVal;
				this->value = this->default;
			}
			else
				throw (EXCEPTION_MAXIMUM_PARAMETERS);
		}
		return true;
	}

	BYTE ByteRecord::GetByteValue()
	{
		return byteValue;
	}

};