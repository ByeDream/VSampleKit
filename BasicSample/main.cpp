#include "Framework.h"

using namespace Framework;

Application g_app;

int main(int argc, const char *argv[])
{
	int ret = g_app.initialize("BasicSample", argc, argv);

	while (true)
	{
		if (g_app.frame())
		{
			break;
		}
	}
	ret = g_app.terminate();
	return 0;
}