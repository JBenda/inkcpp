// inkcpp_cl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>

#include <story.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>
#include <globals.h>

int x(int a, int b)
{
	return a + b;
}

int main()
{
	{
		std::ofstream fout("test.bin", std::ios::binary | std::ios::out);
		ink::compiler::run("test.json", fout);
		fout.close();
	}

	{
		using namespace ink::runtime;

		// Load story
		story* myInk = story::from_file("test.bin");

		// Start runner
		runner thread = myInk->new_runner();

		// Register external functions
		thread->bind(ink::hash_string("brook"), &x);

		// move to knot
		thread->move_to(ink::hash_string("knot_1"));

		while (true)
		{
			while (thread->can_continue())
				std::cout << thread->getline();

			if (thread->has_choices())
			{
				for (const ink::runtime::choice& c : *thread)
				{
					std::cout << "* " << c.text() << std::endl;
				}

				int c = 0;
				std::cin >> c;
				thread->choose(c);
				continue;
			}

			break;
		}

		return 0;
	}
}