/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "config.h"
#ifdef INK_EXPOSE_JSON
#	ifdef INK_ENABLE_UNREAL
#		error Exposing JSON is not supported currently in UE
#	endif
#	include "../json.hpp"
#endif
#include "compilation_results.h"
#include <iostream>

namespace ink
{
/** collection of functions to compile a story.json to story.bin */
namespace compiler
{
	/** file -> file */
	void run(const char* filenameIn, const char* filenameOut, compilation_results* results = nullptr);

	/** file -> stream */
	void run(const char* filenameIn, std::ostream& out, compilation_results* results = nullptr);

#ifdef INK_EXPOSE_JSON
	/** JSON -> file */
	void run(const nlohmann::json&, const char* filenameOut, compilation_results* results = nullptr);

	/** JSON -> stream */
	void run(const nlohmann::json&, std::ostream& out, compilation_results* results = nullptr);
#endif

	/** stream -> stream */
	void run(std::istream& in, std::ostream& out, compilation_results* results = nullptr);

	/** stream -> file */
	void run(std::istream& in, const char* filenameOut, compilation_results* results = nullptr);
} // namespace compiler
} // namespace ink
