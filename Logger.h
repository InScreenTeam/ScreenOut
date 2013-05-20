#pragma once

#include <fstream>

namespace ScreenOut {

    class Logger 
	{
        public:
            enum Level { ERROR, WARNING, INFO };

			Logger(const char *OutfileName = ".log");
            ~Logger (); 
			Logger &operator << (const Level level); 
			
            
            Logger &operator << (const char *text) 
			{

                logger.myFile << text << std::endl;
                return logger;

            }

            Logger (const Logger &) = delete;
            Logger &operator= (const Logger &) = delete;
        private:

            std::ofstream           myFile;

            unsigned int            numWarnings;
            unsigned int            numErrors;

    };

}
#endif // FILELOGGER_HPP