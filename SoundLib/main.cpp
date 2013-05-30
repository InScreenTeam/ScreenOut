#include "stdafx.h";
#include "Sound.h"
#include <list>
#include <algorithm>

using namespace std;
void Output(string str)
{
	cout << str.c_str() << "\n";

}

int main(int argc, char **argv)
{
	Sound* sound;
	if (!(sound = new Sound()))	
		std::cout << "Sound don't created!";
	/*for(int i = 0; i < sound->GetRecordDeviceCount(); ++i)
	{
		Output(sound->GetRecordDeviceName(i));
	}*/
	//sound->RecordStart(10);
	//sound->Test();
	sound->RecordStart(5,-1, "test2.wav");
	Sleep(5000);
	sound->RecordStop();
	return 1;
}	