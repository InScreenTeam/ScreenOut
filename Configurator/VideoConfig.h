#pragma once




#define MAX_FPS (25)
#define MAX_VIDEO_BITRATE (40000000)

using namespace std;

namespace Config
{
	class VideoConfig: BaseConfig
	{
		DWORD fps;
		DWORD bitRate;
	public:
		static const DWORD defaultFps = 10;
		static const DWORD defaultBitRate = 400000;

		VideoConfig();

		void SetFPS(DWORD value);
		void SetBitRate(DWORD value);

		void Set(DWORD fps, DWORD bitRate);
		bool ToVector(vector<Record>& v);
	};
}