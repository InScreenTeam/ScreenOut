#pragma once
#include "stdafx.h"
using namespace std;

namespace Config
{
	class Record
	{
	protected:
		wstring value;
		wstring default;
		
		void Init(wstring comments, wstring section, wstring key, wstring default);
	public:
		wstring comments;
		wstring section;
		wstring key;
		
		Record();
		Record(wstring comments, wstring section, wstring key, wstring default);
		

		wstring GetValue() const;
		wstring GetDefault() const;
		virtual bool SetValue(wstring value);
	};

	

	class DwordRecord: public Record
	{
		DWORD dwordValue;
	public:
		DwordRecord(wstring comments, wstring section,  wstring key, wstring default);
			
		bool SetValue(wstring value);
		bool SetValueInInterval(wstring value, DWORD left, DWORD right);
		bool SetValueInArray(wstring value, const vector<DWORD> & vector);
		DWORD GetDwordValue();
	};

	class ByteRecord: public Record
	{
		BYTE byteValue;
	public:
		ByteRecord(wstring comments, wstring section,  wstring key, wstring default);
		bool SetValueInInterval(wstring value, BYTE left, BYTE right);
		bool SetValueInArray(wstring value, const vector<BYTE> * vector);
		bool SetValue(wstring value);
		BYTE GetByteValue();
	};

}