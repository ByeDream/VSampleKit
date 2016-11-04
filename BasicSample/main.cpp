#include "Framework.h"

using namespace Framework;

Application g_app;

int main(int argc, const char *argv[])
{
	int ret = g_app.initialize("BasicSample", argc, argv);

	while (true)
	{
		// TODO : hard code everything in the application::frame function at the moment, will move the rendering logic to sample later
		if (g_app.frame())
		{
			break;
		}
	}
	ret = g_app.terminate();
	return 0;
}