#include "Util.h"

vector<string> Util::split(string str, char delimiter)
{
	std::vector<string> vs;
	string temp = "";

	for (int i = 0; !(str[i] == '\n'); i++)
	{
		if (str[i] == delimiter) /* 구분자를 만나는 경우 push 후 clear */
		{
			vs.push_back(temp);
			temp.clear();

			continue;
		}

		/* temp에 문자를 하나씩 담는다. */
		temp.push_back(str[i]);
	}

	/* 마지막 string은 push되지 않았으므로 for문 밖에서 push */
	vs.push_back(temp);

	return vs;
}

bool Util::bCanUsestoi(string& str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (!((int)str[i] - '0' >= 0 && (int)str[i] - '0' <= 9))
		{
			return false;
		}
	}

	return true;
}


