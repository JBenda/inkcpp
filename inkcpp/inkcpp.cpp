// inkcpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "compiler.h"
#include "story.h"
#include "runtime.h"
#include "choice.h"

int main()
{
	// Load JSON
	nlohmann::json j;
	try
	{
		std::ifstream fin("test.json");
		fin >> j;
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	{
		std::ofstream fout("test.bin", std::ios::binary | std::ios::out);
		ink::compiler::run(j, fout);
		fout.close();
	}

	{
		// Load ink and start a runner
		ink::runtime::story myInk("test.bin");
		ink::runtime::runner thread(&myInk);

		while (true)
		{
			while (thread)
				std::cout << thread;

			if(thread.has_choices())
			{
				for (const ink::runtime::choice& c : thread)
				{
					std::cout << "* " << c.text() << std::endl;
				}

				int c = 0;
				std::cin >> c;
				thread.choose(c);
				continue;
			}

			break;
		}

		return 0;
	}

	/*{
		binary::runtime::ink i;
		i.load("test.bin");

		binary::runtime::runner r(&i);
		
		while (true)
		{
			r.run();
			for (auto c : r.choices())
			{
				std::cout << "* " << c->text() << std::endl;
			}
			
			int index;
			std::cin >> index;
			r.choose(index);
			r.run();
		}
	}*/

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
