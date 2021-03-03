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

#include "test.h"

void usage()
{
	using namespace std;
	cout
		<< "Usage: inkcpp_cl <options> [<json file>]\n"
		<< "\t-p:\tPlay mode\n"
		<< "\t--no-compile:\tdon't compile the json file\n"
		<< "\t-b <filename>:\tIn/Output filename InkBin\n"
		<< "\t-s <filename>:\tIn/Output filename StringList\n"
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
	std::string inkbinFilename;
	std::string stringListFilename;
	bool playMode = false, testMode = false, testDirectory = false;
	bool compileMode = true;
	for (int i = 1; i < argc - (compileMode ? 1 : 0); i++)
	{
		std::string option = argv[i];
		if (option == "-b")
		{
			inkbinFilename = argv[i + 1];
			i += 1;
		}
		else if (option == "-s")
		{
			stringListFilename = argv[i+1];
			i += 1;
		}
		else if (option == "-p")
			playMode = true;
		else if (option == "-t")
			testMode = true;
		else if (option == "-td")
		{
			testMode = true;
			testDirectory = true;
		}
		else if (option == "--no-compile")
		{ compileMode = false; }
		else
		{
			std::cerr << "Unrecognized option: '" << option << "'\n";
		}
	}

	// Get input filename
	std::string inputFilename = argv[argc - 1];
	if (inputFilename == "--no-compile") {
		inputFilename = "";
		compileMode = false;
	}

	// Test mode
	if (testMode)
	{
		bool result;
		if (testDirectory)
			result = test_directory(inputFilename);
		else
			result = test(inputFilename);

		return result ? 0 : -1;
	}

	// If output filename not specified, use input filename as guideline
	if (inkbinFilename.empty())
	{
		inkbinFilename = std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".bin");
	}
	if (stringListFilename.empty()) {
		stringListFilename = inkbinFilename+".str";
	}

	if (compileMode) {
		// If input filename is an .ink file
		int val = inputFilename.find(".ink");
		if (val == inputFilename.length() - 4)
		{
			// Create temporary filename
			std::string jsonFile = std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".tmp");

			// Then we need to do a compilation with inklecate
			try
			{
				inklecate(inputFilename, jsonFile);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Inklecate Error: " << e.what() << std::endl;
				return 1;
			}

			// New input is the json file
			inputFilename = jsonFile;
		}

		// Open file and compile
		try
		{
			ink::compiler::compilation_results results;
			ink::compiler::run(
					inputFilename.c_str(),
					inkbinFilename.c_str(),
					stringListFilename.c_str(),
					&results);

			// Report errors
			for (auto& warn : results.warnings)
				std::cerr << "WARNING: " << warn << '\n';
			for (auto& err : results.errors)
				std::cerr << "ERROR: " << err << '\n';

			if (results.errors.size() > 0 && playMode)
			{
				std::cerr << "Cancelling play mode. Errors detected in compilation" << std::endl;
				return -1;
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "Unhandled InkBin compiler exception: " << e.what() << std::endl;
			return 1;
		}
	}

	if (!playMode)
		return 0;

	// Run the story
	try
	{
		using namespace ink::runtime;

		// Load story
		story* myInk = story::create(inkbinFilename.c_str(), stringListFilename.c_str());

		// Start runner
		runner thread = myInk->new_runner();

		while (true)
		{
			while (thread->can_continue())
				std::cout << thread->getline();

			if (thread->has_choices())
			{
				// Extra end line
				std::cout << std::endl;

				int index = 1;
				for (const ink::runtime::choice& c : *thread)
				{
					std::cout << index++ << ": " << c.text() << std::endl;
				}

				int c = 0;
				std::cin >> c;
				thread->choose(c - 1);
				std::cout << "?> ";
				continue;
			}

			// out of content
			break;
		}

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Unhandled ink runtime exception: " << e.what() << std::endl;
		return 1;
	}
}
