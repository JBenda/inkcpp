#pragma once

#include <vector>
#include <string>

namespace ink::compiler
{
/** list of errors/warnings */
typedef std::vector<std::string> error_list;

/** stores results from the compilation process */
struct compilation_results {
	error_list warnings; ///< list of all warnings generated
	error_list errors;   ///< list of all errors generated
};
} // namespace ink::compiler
