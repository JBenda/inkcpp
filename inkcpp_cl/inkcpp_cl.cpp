// inkcpp_cl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <regex>

#include <story.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>
#include <globals.h>
#include <compiler2.h>

void usage()
{
	using namespace std;
	cout
		<< "Usage: inkcpp_cl <options> <json file>\n"
		<< "\t-o <filename>:\tOutput file name\n"
		<< "\t-p:\tPlay mode\n"
		<< endl;
}

int main(int argc, const char** argv)
{
	// Usage
	if (argc == 1)
	{
		usage();
		return 1;
	}

	// Parse options
	std::string outputFilename;
	bool playMode = false;
	for (int i = 1; i < argc - 1; i++)
	{
		std::string option = argv[i];
		if (option == "-o")
		{
			outputFilename = argv[i + 1];
			i += 1;
		}
		else if (option == "-p")
			playMode = true;
		else
		{
			std::cerr << "Unrecognized option '" << option << "'\n";
		}
	}

	// Get input filename
	std::string inputFilename = argv[argc - 1];

	// If output filename not specified, use input filename as guideline
	if (outputFilename.empty())
	{
		outputFilename = std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".bin");
	}

	// Open file and compile
	{
		//std::ofstream fout(outputFilename, std::ios::binary | std::ios::out);
		//ink::compiler::run(inputFilename.c_str(), fout);
		//fout.close();
		ink::compiler::run_new(inputFilename.c_str(), outputFilename.c_str());
	}

	if (!playMode)
		return 0;

	// Run the story
	{
		using namespace ink::runtime;

		// Load story
		story* myInk = story::from_file(outputFilename.c_str());

		// Start runner
		runner thread = myInk->new_runner();

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

			// out of content
			break;
		}

		return 0;
	}
}