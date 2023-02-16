#pragma once

#include <vector>
#include <string>
#include <ctime>

using namespace std;

namespace Util
{
	vector<string> split(string& str, char delimiter);
	bool bCanUsestoi(string& str);

	struct tm
	{
		tm()
		{
			tm_sec = 0;
			tm_min = 0;
			tm_hour = 0;
			tm_mday = 0;
			tm_mon = 0;
			tm_year = 0;
			tm_wday = 0;
			tm_yday = 0;
			tm_isdst = 0;
		}

		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
		int tm_wday;
		int tm_yday;
		int tm_isdst;
	};
}