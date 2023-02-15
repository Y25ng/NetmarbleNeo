#include "Util.h"

vector<string> Util::split(string str, char delimiter)
{
	std::vector<string> vs;
	string temp = "";

	for (int i = 0; !(str[i] == '\n'); i++)
	{
		if (str[i] == delimiter) /* �����ڸ� ������ ��� push �� clear */
		{
			vs.push_back(temp);
			temp.clear();

			continue;
		}

		/* temp�� ���ڸ� �ϳ��� ��´�. */
		temp.push_back(str[i]);
	}

	/* ������ string�� push���� �ʾ����Ƿ� for�� �ۿ��� push */
	vs.push_back(temp);

	return vs;
}


