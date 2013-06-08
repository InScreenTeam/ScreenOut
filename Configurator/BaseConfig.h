#pragma once

#include "Record.h"
using namespace std;

namespace Config
{
	typedef map<wstring, map<wstring, Record>> recordMap;
	
	class BaseConfig
	{
	public:
		recordMap records;

		BaseConfig(void);
		~BaseConfig(void);
	};

}