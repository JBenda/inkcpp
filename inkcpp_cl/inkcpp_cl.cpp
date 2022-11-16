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
#include <snapshot.h>

#include "test.h"

void usage()
{
	using namespace std;
	cout
		<< "Usage: inkcpp_cl <options> <json file>\n"
		<< "\t-o <filename>:\tOutput file name\n"
		<< "\t-p [<snapshot_file>]:\tPlay mode\n\toptional snapshot file to load\n\tto create a snapshot file enter '-1' as choice\n"
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
	bool playMode = false, testMode = false, testDirectory = false;
	std::string snapshotFile;
	for (int i = 1; i < argc - 1; i++)
	{
		std::string option = argv[i];
		if (option == "-o")
		{
			outputFilename = argv[i + 1];
			i += 1;
		}
		else if (option == "-p") {
			playMode = true;
			if (i + 1 < argc - 1 && argv[i+1][0] != '-') {
				++i;
				snapshotFile = argv[i];
			}
		}
		else if (option == "-t")
			testMode = true;
		else if (option == "-td")
		{
			testMode = true;
			testDirectory = true;
		}
		else
		{
			std::cerr << "Unrecognized option: '" << option << "'\n";
		}
	}

	// Get input filename
	std::string inputFilename = argv[argc - 1];

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
	if (outputFilename.empty())
	{
		outputFilename = std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".bin");
	}

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
		std::ofstream fout(outputFilename, std::ios::binary | std::ios::out);
		ink::compiler::run(inputFilename.c_str(), fout, &results);
		fout.close();

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

	if (!playMode)
		return 0;

	// Run the story
	try
	{
		using namespace ink::runtime;

		// Load story
		story* myInk = story::from_file(outputFilename.c_str());

		// Start runner
		runner thread;
		if (snapshotFile.size()) {
			auto snap_ptr = snapshot::from_file( snapshotFile.c_str() );
			thread = myInk->new_runner_from_snapshot(*snap_ptr);
			delete snap_ptr;
		} else {
			thread = myInk->new_runner();
		}

		while (true)
		{
			while (thread->can_continue())
				std::cout << thread->getline();
			if (thread->has_tags()){
				std::cout << "# tags: ";
				for (int i = 0; i < thread->num_tags(); ++i) {
					if(i != 0) std::cout << ", ";
					std::cout << thread->get_tag(i);
				}
				std::cout << std::endl;
			}
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
				if (c == -1) {
					snapshot* snap = thread->create_snapshot();
					snap->write_to_file(std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".snap").c_str());
					delete snap;
					break;
				}
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
