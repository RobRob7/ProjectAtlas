#include "application.h"

// #include <iostream>

// window settings
unsigned int width = 1600;
unsigned int height = 1200;
const char* windowName = "Atlas";

// main driver
int main()
{
	try
	{
		Application app(width, height, windowName);
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
} // end of main
