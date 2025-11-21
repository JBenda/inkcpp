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

#include "config.h"
#include "test.h"
#include "types.h"

size_t depth = 0;

std::ostream& operator<<(std::ostream& os, const ink::config::statistics::container& c)
{
	os << "(" << c.size << "/" << c.capacity << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const ink::config::statistics::list_table& lt)
{
	os << "\n";
	depth += 1;
	os << std::string(depth, '\t') << "editable_lists" << lt.editable_lists << "\n";
	os << std::string(depth, '\t') << "list_types" << lt.list_types << "\n";
	os << std::string(depth, '\t') << "flags" << lt.flags << "\n";
	os << std::string(depth, '\t') << "lists" << lt.lists << "\n";
	depth -= 1;
	return os;
}

std::ostream& operator<<(std::ostream& os, const ink::config::statistics::string_table& st)
{
	os << "\n";
	depth += 1;
	os << std::string(depth, '\t') << "string_refs" << st.string_refs << "\n";
	depth -= 1;
	return os;
}

std::ostream& operator<<(std::ostream& os, const ink::config::statistics::runner& r)
{
	os << "\n";
	depth += 1;
	os << std::string(depth, '\t') << "threads" << r.threads << "\n";
	os << std::string(depth, '\t') << "evaluation_stack" << r.evaluation_stack << "\n";
	os << std::string(depth, '\t') << "container_stack" << r.container_stack << "\n";
	os << std::string(depth, '\t') << "active_tags" << r.active_tags << "\n";
	os << std::string(depth, '\t') << "runtime_stack" << r.runtime_stack << "\n";
	os << std::string(depth, '\t') << "runtime_ref_stack" << r.runtime_ref_stack << "\n";
	os << std::string(depth, '\t') << "output" << r.output << "\n";
	os << std::string(depth, '\t') << "choices" << r.choices << "\n";
	depth -= 1;
	return os;
}

std::ostream& operator<<(std::ostream& os, const ink::config::statistics::global& g)
{
	os << "\n";
	depth += 1;
	os << std::string(depth, '\t') << "variables" << g.variables << "\n";
	os << std::string(depth, '\t') << "variables_observers" << g.variables_observers << "\n";
	os << std::string(depth, '\t') << "lists" << g.lists;
	os << std::string(depth, '\t') << "strings" << g.strings;
	depth -= 1;
	return os;
}

void usage()
{
	using namespace std;
	cout << "Usage: inkcpp_cl <options> <json file>\n"
	     << "\t-o <filename>:\tOutput file name\n"
	     << "\t-p [<snapshot_file>]:\tPlay mode\n\toptional snapshot file to load\n\tto create a "
	        "snapshot file enter '-1' as choice\n"
	     << "\t--ommit-choice-tags:\tdo not print tags after choices, primarly used to be compatible "
	        "with inkclecat output"
	     << "\t--inklecate <path-to-inklecate>:\toverwrites INKLECATE enviroment variable\n"
	     << "\t--statistics:\tprints memory statistics before each choice\n"
	     << endl;
}

int main(int argc, const char** argv)
{
	// Usage
	if (argc == 1) {
		usage();
		return 1;
	}

	// Parse options
	std::string outputFilename;
	bool        playMode = false, testMode = false, testDirectory = false, ommit_choice_tags = false;
	std::string snapshotFile;
	bool        show_statistics    = false;
	const char* inklecateOverwrite = nullptr;
	for (int i = 1; i < argc - 1; i++) {
		std::string option = argv[i];
		if (option == "-o") {
			outputFilename = argv[i + 1];
			i += 1;
		} else if (option == "-p") {
			playMode = true;
			if (i + 1 < argc - 1 && argv[i + 1][0] != '-') {
				++i;
				snapshotFile = argv[i];
			}
		} else if (option == "--ommit-choice-tags") {
			ommit_choice_tags = true;
		} else if (option == "-t") {
			testMode = true;
		} else if (option == "-td") {
			testMode      = true;
			testDirectory = true;
		} else if (option == "--inklecate") {
			if (i + 1 < argc - 1 && argv[i + 1][0] != '-') {
				++i;
				inklecateOverwrite = argv[i];
			}
		} else if (option == "--statistics") {
			show_statistics = true;
		} else {
			std::cerr << "Unrecognized option: '" << option << "'\n";
		}
	}

	// Get input filename
	std::string inputFilename = argv[argc - 1];

	// Test mode
	// if (testMode) {
	// 	bool result;
	// 	if (testDirectory) {
	// 		result = test_directory(inputFilename);
	// 	} else {
	// 		result = test(inputFilename);
	// 	}

	// 	return result ? 0 : -1;
	// }

	// If output filename not specified, use input filename as guideline
	if (outputFilename.empty()) {
		outputFilename = std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".bin");
	}

	// If input filename is an .ink file
	size_t  val                   = inputFilename.find(".ink");
	bool json_file_is_tmp_file = false;
	if (val == inputFilename.length() - 4) {
		// Create temporary filename
		std::string jsonFile = std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".tmp");

		// Then we need to do a compilation with inklecate
		try {
			inklecate(inputFilename, jsonFile, inklecateOverwrite);
		} catch (const std::exception& e) {
			std::cerr << "Inklecate Error: " << e.what() << std::endl;
			return 1;
		}

		// New input is the json file
		json_file_is_tmp_file = true;
		inputFilename         = jsonFile;
	}

	// Open file and compile
	try {
		ink::compiler::compilation_results results;
		std::ofstream                      fout(outputFilename, std::ios::binary | std::ios::out);
		ink::compiler::run(inputFilename.c_str(), fout, &results);
		fout.close();
		if (json_file_is_tmp_file) {
			remove(inputFilename.c_str());
		}

		// Report errors
		for (auto& warn : results.warnings) {
			std::cerr << "WARNING: " << warn << '\n';
		}
		for (auto& err : results.errors) {
			std::cerr << "ERROR: " << err << '\n';
		}

		if (results.errors.size() > 0 && playMode) {
			std::cerr << "Cancelling play mode. Errors detected in compilation" << std::endl;
			return -1;
		}
	} catch (std::exception& e) {
		if (json_file_is_tmp_file) {
			remove(inputFilename.c_str());
		}
		std::cerr << "Unhandled InkBin compiler exception: " << e.what() << std::endl;
		return 1;
	}

	if (! playMode) {
		return 0;
	}

	// Run the story
	try {
		using namespace ink::runtime;

		// Load story
		std::unique_ptr<story> myInk{story::from_file(outputFilename.c_str())};

		// Start runner
		runner  thread;
		globals variables;
		if (snapshotFile.size()) {
			auto snap_ptr = snapshot::from_file(snapshotFile.c_str());
			thread        = myInk->new_runner_from_snapshot(*snap_ptr);
			delete snap_ptr;
		} else {
			variables = myInk->new_globals();
			thread    = myInk->new_runner(variables);
		}

		while (true) {
			while (thread->can_continue()) {
				std::cout << thread->getline();
				if (thread->has_tags()) {
					std::cout << "# tags: ";
					for (ink::size_t i = 0; i < thread->num_tags(); ++i) {
						if (i != 0) {
							std::cout << ", ";
						}
						std::cout << thread->get_tag(i);
					}
					std::cout << std::endl;
				}
			}
			if (thread->has_choices()) {
				// Extra end line
				std::cout << std::endl;

				int index = 1;
				for (const ink::runtime::choice& c : *thread) {
					std::cout << index++ << ": " << c.text();
					if (! ommit_choice_tags && c.has_tags()) {
						std::cout << "\n\t";
						for (ink::size_t i = 0; i < c.num_tags(); ++i) {
							std::cout << "# " << c.get_tag(i) << " ";
						}
					}
					std::cout << std::endl;
				}

				if (show_statistics) {
					std::cout << "runner:" << thread->statistics() << "globals:" << variables->statistics()
					          << std::endl;
				}

				int c = 0;
				std::cout << "?> ";
				std::cin >> c;
				if (c == -1) {
					snapshot* snap = thread->create_snapshot();
					snap->write_to_file(
					    std::regex_replace(inputFilename, std::regex("\\.[^\\.]+$"), ".snap").c_str()
					);
					delete snap;
					break;
				}
				thread->choose(c - 1);
				continue;
			}

			// out of content
			break;
		}
	} catch (const std::exception& e) {
		std::cerr << "Unhandled ink runtime exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
