#include "Logger.h"
#include <ctime>

namespace ScreenOut
{

	Logger::Logger(const char *OutfileName)
	{
		Outfile.open (OutfileName);                
	}


	Logger::~Logger(void)
	{					
		if (Outfile.is_open()) 
		{						
			Outfile << std::endl << std::endl;                 												
			Outfile.close();
		}

	}

	Logger& operator<<(Logger& logger, const Logger::Level level)
	{
		logger.Outfile << std::endl;
		logger.LogTime();
        switch (level) 
		{
            case Logger::Level::LOG_ERROR:
				logger.Outfile <<"[ERROR]: ";                        
                break;

            case Logger::Level::LOG_WARNING:
				logger.Outfile << "[WARNING]: ";                        
                break;

            default:
				logger.Outfile << "[INFO]: ";
                break;
        }
		return logger;
	}

	Logger& operator<<(Logger& logger, const char* text)
	{
		logger.Outfile << text;
		return logger;
	}

	Logger &operator<< (Logger& logger, int num)
	{
		logger.Outfile << " " << num << " ";
		return logger;
	}

	Logger &operator<< (Logger& logger, double num)
	{
		logger.Outfile << " " << num << " ";
		return logger;
	}

	void Logger::LogTime()
	{
		time_t currentTime = time(0);
		struct tm* localTime = localtime(&currentTime);
		Outfile << localTime->tm_mday << "/"
			<< localTime->tm_mon + 1 << "/"
			<< localTime->tm_year + 1900 <<" "
			<< localTime->tm_hour << ":"
			<< localTime->tm_min << ":"
			<< localTime->tm_sec<< " ";
	}
}