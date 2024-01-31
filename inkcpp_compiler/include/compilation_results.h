/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
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
