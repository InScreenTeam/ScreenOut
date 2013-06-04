#pragma once
#define FILE_NAME "config"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <map>
#include "ConfigClasses.h"
using namespace std;
using namespace Config;

class Configurator
{
public:
	enum CommentChar
	{
		Pound = '#',
		SemiColon = ';'
	};

	AudioConfig audioConfig;
	VideoConfig videoConfig;

	Configurator(void);
	virtual ~Configurator(void);

	 bool AddSection(string SectionName);
	 bool CommentRecord(CommentChar cc, string KeyName,string SectionName);
	 bool CommentSection(char CommentChar, string SectionName);
	 string Content(string FileName);
	 bool Create(string FileName);
	 bool DeleteRecord(string KeyName, string SectionName);
	 bool DeleteSection(string SectionName);
	 vector<Record> GetRecord(string KeyName, string SectionName);
	 vector<Record> GetSectionVector(string SectionName);
	 map<string, string> GetSectionMap(string SectionName);
	 vector<string> GetSectionNames(string FileName);
	 string GetValue(string KeyName, string SectionName);
	 bool RecordExists(string KeyName, string SectionName);
	 bool RenameSection(string OldSectionName, string NewSectionName);
	 bool SectionExists(string SectionName);
	 bool SetRecordComments(string Comments, string KeyName, string SectionName);
	 bool SetSectionComments(string Comments, string SectionName);
	 bool SetValue(string KeyName, string Value, string SectionName);
	 bool Sort(string FileName, bool Descending);
	 bool UnCommentRecord(string KeyName,string SectionName);
	 bool UnCommentSection(string SectionName);
	
	 bool WriteDefault();

private:
	string fileName;

	 vector<Record> GetSections(string FileName);
	 bool Load(vector<Record>& content);
	 bool Save(vector<Record>& content);

	struct RecordSectionIs : std::unary_function<Record, bool>
	{
		std::string section_;

		RecordSectionIs(const std::string& section): section_(section){}

		bool operator()( const Record& rec ) const
		{
			return rec.Section == section_;
		}
	};
	struct RecordSectionKeyIs : std::unary_function<Record, bool>
	{
		std::string section_;
		std::string key_;

		RecordSectionKeyIs(const std::string& section, const std::string& key): section_(section),key_(key){}

		bool operator()( const Record& rec ) const
		{
			return ((rec.Section == section_)&&(rec.Key == key_));
		}
	};
	struct AscendingSectionSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Section < End.Section;
		}
	};
	struct DescendingSectionSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Section > End.Section;
		}
	};
	struct AscendingRecordSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Key < End.Key;
		}
	};
	struct DescendingRecordSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Key > End.Key;
		}
	};
};