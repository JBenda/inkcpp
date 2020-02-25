#pragma once

#include <vector>
#include <string>

namespace ink::compiler
{
	typedef std::vector<std::string> error_list;

	// stores results from the compilation process
	struct compilation_results
	{
		error_list warnings;
		error_list errors;
	};
}