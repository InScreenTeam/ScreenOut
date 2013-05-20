#include "Logger.h"

namespace ScreenOut
{

	Logger::Logger(const char *OutfileName)
	{
					myFile.open (OutfileName);                
					if (myFile.is_open()) {                    
						myFile << "Log file created" << std::endl << std::endl;
					}

				}


	Logger::~Logger(void)
	{
					if (myFile.is_open()) {
						myFile << std::endl << std::endl;                 
						myFile << numWarnings << " warnings" << std::endl;
						myFile << numErrors << " errors" << std::endl;
						myFile.close();
					}

	}
	Logger &Logger::operator<<(const Logger::Level level)
	{

                switch (level) {
                    case Logger::Level::ERROR:
                        myFile << "[ERROR]: ";
                        ++numErrors;
                        break;

                    case Logger::Level::WARNING:
                        myFile << "[WARNING]: ";
                        ++logger.numWarnings;
                        break;

                    default:
                        myFile << "[INFO]: ";
                        break;
                }
                return this;

            }
}