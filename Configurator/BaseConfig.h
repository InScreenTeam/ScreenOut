#pragma once

#include "Record.h"

namespace Config
{

	class BaseConfig
	{
	public:
		BaseConfig(void);
		~BaseConfig(void);

		virtual bool ToVector(vector<Record>& v);
	};

}