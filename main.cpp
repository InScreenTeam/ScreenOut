#include "Recorder.h"
#include <boost\thread.hpp>

using namespace boost;

int main(void)
{
	ScreenOut::Recorder* recorder = new ScreenOut::Recorder();
	recorder->Start();
	
	//cin.get();
	Sleep(20000);
	recorder->Stop();
	while(true)
	{
		this_thread::sleep_for(chrono::nanoseconds(chrono::milliseconds(50)));
		if (recorder->IsDone())
			break;
	}
	//cin.get();
 }
