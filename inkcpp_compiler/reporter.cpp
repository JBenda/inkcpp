#include "reporter.h"
#include "compilation_results.h"

#include <iostream>

namespace ink::compiler::internal
{
	reporter::reporter()
		: _results(nullptr)
	{
	}

	void reporter::set_results(compilation_results* results)
	{
		_results = results;
	}

	void reporter::clear_results()
	{
		_results = nullptr;
	}

	std::ostream& reporter::warn()
	{
		// TODO: insert return statement here
		return std::cerr;
	}

	std::ostream& reporter::err()
	{
		// TODO: insert return statement here
		return std::cerr;
	}

	std::ostream& reporter::crit()
	{
		// TODO: insert return statement here
		return std::cerr;
	}
}