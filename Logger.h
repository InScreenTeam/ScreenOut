#pragma once

#include <fstream>

namespace ScreenOut {

    class Logger 
	{
        public:
            enum Level { LOG_ERROR, LOG_WARNING, LOG_INFO };

			Logger(const char *OutfileName = ".log");
            ~Logger(); 
			friend Logger &operator<< (Logger& logger, const Level level);
			friend Logger &operator<< (Logger& logger, const char* text);
			friend Logger &operator<< (Logger& logger, int num);
			friend Logger &operator<< (Logger& logger, double num);
		private:
			void LogTime();
        private:
            std::ofstream           Outfile;
    };

}