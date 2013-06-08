#pragma once

#include "Record.h"




#define FILE_NAME "config"

#include "Record.h"
#include "BaseConfig.h"

using namespace std;
using namespace Config;

class Configurator
{
private:
	wstring fileName;

public:
	enum CommentChar
	{
		Pound = '#',
		SemiColon = ';'
	};

	BaseConfig baseConfig;
	
	Configurator(void);
	virtual ~Configurator(void);

	 //bool WriteDefault();
	bool Load(recordMap & records);
	bool Save(recordMap & records);
	bool WriteDefault();

	bool SetValue(wstring section, wstring key, wstring value);
	wstring GetValue(wstring section, wstring key);
	Record GetRecord(wstring section, wstring key);
};