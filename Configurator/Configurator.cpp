#include "stdafx.h"

#include <fstream>
#include <algorithm>
#include <functional>

#include "BaseConfig.h"
#include "Record.h"
#include "Configurator.h"


using namespace Config;

Configurator::Configurator(void)												
{
	fileName = wstring(TEXT(FILE_NAME));
	Load(baseConfig.records);
//	baseConfig.records[L"Video"][L"FPS"].SetValueInInterval(L"5",1,4);
}

Configurator::~Configurator(void)
{
	Save(baseConfig.records);
}

// A function to trim whitespace from both sides of a given string
void Trim(std::wstring& str, const std::wstring & ChrsToTrim = L" \t\n\r", int TrimDir = 0)
{
	size_t startIndex = str.find_first_not_of(ChrsToTrim);
	if (startIndex == std::string::npos){str.erase(); return;}
	if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
	if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
}

bool Configurator::Load(recordMap & records)
{
	wstring s;																
	wstring CurrentSection;													

	wifstream inFile (fileName.c_str());									
	if (!inFile.is_open())
	{
		WriteDefault();
		return false;
	}									

	wstring comments = L"";													

	while(!std::getline(inFile, s).eof())									
	{
		Trim(s);															
		if(!s.empty())														
		{
			Record r;														

			if((s[0]=='#')||(s[0]==';'))									// Is this a commented line?
			{
				if ((s.find('[')==string::npos)&&							// If there is no [ or =
					(s.find('=')==string::npos))							// Then it's a comment
				{
					comments += s + L'\n';									// Add the comment to the current comments string
				} 
					s.erase(s.begin());										// Remove the comment for further processing
					Trim(s);
				}// Remove any more whitespace

			if(s.find('[')!=string::npos)									// Is this line a section?
			{		
				s.erase(s.begin());											// Erase the leading bracket
				s.erase(s.find(']'));										// Erase the trailing bracket
				r.section = s;												// Set the Section value
				r.key = L"";													// Set the Key value
				r.SetValue(L"");							
				CurrentSection = s;
			}

			if(s.find('=')!=string::npos)									// Is this line a Key/Value?
			{
				r.comments = comments;										// Add the comments string (if any)
				comments = L"";												// Clear the comments for re-use
				r.section = CurrentSection;									// Set the section to the current Section
				r.key = s.substr(0,s.find('='));							// Set the Key value to everything before the = sign
				r.SetValue(s.substr(s.find('=')+1));							// Set the Value to everything after the = sign
			}
			if(comments == L"")												// Don't add a record yet if its a comment line
				records[r.section][r.key] = r;
		}
	}

	inFile.close();															// Close the file
	return true;
}

bool Configurator::Save(recordMap & records)
{
	wofstream outFile (fileName.c_str());									
		if (!outFile.is_open()) 
			return false;
	for (recordMap::iterator i = records.begin(); i != records.end(); i++)
	{
		wstring section = i->first;
		for (map <wstring, Record>::iterator j = i->second.begin(); j != i->second.end(); j++)							
		{
			Record record = j->second;
			if (record.comments != L"")
				outFile << record.comments << endl;		
			if(record.key == L"")											
				outFile << "[" << record.section << "]" << endl;							
			else
				outFile << record.key << L"=" << record.GetValue() << endl;								
		}
	}
									
	outFile.close();														
	return true;
}

bool Configurator::WriteDefault()
{
	BaseConfig tempBaseConfig;
	Save(tempBaseConfig.records);
	return true;
}

bool Configurator::SetValue(wstring section, wstring key, wstring value)
{
	baseConfig.records[section][key].SetValue(value);
	return true;
}

wstring Configurator::GetValue(wstring section, wstring key)
{
	return baseConfig.records[section][key].GetValue();
}

Record Configurator::GetRecord(wstring section, wstring key)
{
	return baseConfig.records[section][key];
}