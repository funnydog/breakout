#include <iostream>

#include "game.hpp"

int main(int argc, char *argv[])
{
	if (argc != 1)
	{
		std::cerr << argv[0] << " takes no arguments.\n";
		return 1;
	}

	try
	{
		Game game;
		game.run();
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return 1;
	}
}
