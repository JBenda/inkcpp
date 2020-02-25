#include "compiler.h"

#include "json_compiler.h"
#include "binary_emitter.h"

#include <fstream>

namespace ink::compiler
{
	void run(const nlohmann::json& src, std::ostream& out, compilation_results* results)
	{
		using namespace internal;
		
		// Create compiler and emitter
		json_compiler compiler;
		binary_emitter emitter;

		// Compile into emitter
		compiler.compile(src, &emitter, results);

		// write emitter's results into the stream
		emitter.output(out);
	}

	void run(const char* filenameIn, const char* filenameOut, compilation_results* results)
	{
		// Load JSON
		nlohmann::json j;
		std::ifstream fin(filenameIn);
		fin >> j;

		// Open output stream
		std::ofstream fout(filenameOut, std::ios::binary | std::ios::out);

		// Run compiler
		ink::compiler::run(j, fout, results);

		// Close file
		fout.close();
	}

	void run(const char* filenameIn, std::ostream& out, compilation_results* results)
	{
		// Load JSON
		nlohmann::json j;
		std::ifstream fin(filenameIn);
		fin >> j;

		// Run compiler
		ink::compiler::run(j, out, results);
	}

	void run(const nlohmann::json& j, const char* filenameOut, compilation_results* results)
	{
		// Open output stream
		std::ofstream fout(filenameOut, std::ios::binary | std::ios::out);

		// Run compiler
		ink::compiler::run(j, fout, results);

		// Close file
		fout.close();
	}

	void run(std::istream& in, std::ostream& out, compilation_results* results)
	{
		// Load JSON
		nlohmann::json j;
		in >> j;

		// Run compiler
		ink::compiler::run(j, out, results);
	}

	void run(std::istream& in, const char* filenameOut, compilation_results* results)
	{
		// Load JSON
		nlohmann::json j;
		in >> j;

		// Open output stream
		std::ofstream fout(filenameOut, std::ios::binary | std::ios::out);

		// Run compiler
		ink::compiler::run(j, fout, results);

		// Close file
		fout.close();
	}
}