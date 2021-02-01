#include "test.h"

#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>

#include <globals.h>
#include <runner.h>
#include <story.h>
#include <compiler.h>
#include <choice.h>

void inklecate(const std::string& inkFilename, const std::string& jsonFilename)
{
	std::string command = "inklecate -o " + jsonFilename + " " + inkFilename;
	std::system(command.c_str());
}

void load_file(const std::string& filename, std::string& result)
{
	// Open file
	std::ifstream file(filename.c_str());

	// Get size
	file.seekg(0, std::ios::end);
	result.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	// Load into string
	result.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	// Close
	file.close();
}

bool test(const std::string& inkFilename)
{
	using namespace ink::runtime;

	std::cout << std::filesystem::path(inkFilename).filename().string() << std::endl;

	// Compile into a temporary json file
	inklecate(inkFilename, "test.tmp");

	// Compile into binary
	ink::compiler::compilation_results results;
	ink::compiler::run("test.tmp", "test.bin", &results);

	std::vector<std::string> expectations;
	std::vector<int> choices;
	std::vector<std::string> choiceStrings;

	{
		// Load entire ink file into memory
		std::string inkFile;
		load_file(inkFilename, inkFile);

		// Check for ignore
		if (inkFile.find("//IGNORE") == 0)
		{
			std::cout << "IGNORED" << std::endl;
			return true;
		}

		// Check for compile errors post ignore
		if (results.errors.size() > 0)
		{
			for (auto& error : results.errors)
			{
				std::cerr << "ERROR: " << error << std::endl;
			}
			return false;
		}

		// Load expectations
		auto reg = std::regex("\\/\\*\\*[ \t]*(\\d+(:.+)?)?[ \t]*\n((.|\n)*?)\\*\\*\\/");
		auto iter = std::sregex_iterator(inkFile.begin(), inkFile.end(), reg);
		auto end = std::sregex_iterator();

		// Go through them
		for (; iter != end; ++iter)
		{
			if ((*iter)[1].matched)
			{
				expectations.push_back((*iter)[3].str());
				choices.push_back(atoi((*iter)[1].str().c_str()));
				
				if ((*iter)[2].matched)
					choiceStrings.push_back((*iter)[2].str().substr(1));
				else
					choiceStrings.push_back("");
			}
			else
			{
				expectations.push_back((*iter)[3].str());
			}
		}

		std::reverse(expectations.begin(), expectations.end());
		std::reverse(choices.begin(), choices.end());
	}

	// Load story
	auto file = story::from_file("test.bin");
	auto runner = file->new_runner();

	while (true)
	{
		// Run continuously
		std::string output = runner->getall();

		// Check against expectatins
		if (expectations.size() == 0)
		{
			std::cout << "FAIL: Extra content detected in ink file:\n"
				<< output;
			return false;
		}

		// Check against expectations
		std::string expect = *expectations.rbegin();
		expectations.pop_back();
		bool success = output == expect;
		if (!success)
		{
			std::cout << "FAIL: Mismatch\n";
			std::cout << "== Expected ==\n"
				<< expect;
			std::cout << "\n== Actual ==\n"
				<< output;
			return false;
		}

		if (runner->has_choices())
		{
			if (choices.size() == 0)
			{
				std::cout << "FAIL: Encountered choice without choice index" << std::endl;
				return false;
			}

			// Pick choice
			int choice = *choices.rbegin();
			choices.pop_back();

			// Make sure text matches
			std::string choiceStr = *choiceStrings.rbegin();
			choiceStrings.pop_back();
			if (choiceStr != "" && runner->get_choice(choice)->text() != choiceStr)
			{
				std::cout << "FAIL: CHOICE MISMATCH\n";

				std::cout << "== Expected ==\n"
					<< choiceStr << "\n"
					<< "== Actual ==\n"
					<< runner->get_choice(choice)->text() << std::endl;
				return false;
			}

			runner->choose(choice);
		}
		else
			break;
	}

	std::cout << "SUCCESS!" << std::endl;
	return true;
}

bool test_directory(const std::string& directory)
{
	for (auto p : std::filesystem::directory_iterator(directory))
	{
		if (p.path().extension() == ".ink")
		{
			bool success = test(p.path().string());
			if (!success)
				return false;
		}
	}

	return true;
}
