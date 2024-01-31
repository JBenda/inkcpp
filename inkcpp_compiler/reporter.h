/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "compilation_results.h"
#include <sstream>

namespace ink::compiler::internal
{
	class error_strbuf : public std::stringbuf
	{
	public:
		// start a new error message to be outputted to a given list
		void start(error_list* list);

		// If set, the next sync will throw an exception
		void throw_on_sync(bool);
	protected:
		virtual int sync() override;

	private:
		error_list* _list = nullptr;
		bool _throw = false;
	};

	class reporter
	{
	protected:
		reporter();
		virtual ~reporter() { }

		// sets the results pointer for this reporter
		void set_results(compilation_results*);

		// clears the results pointer
		void clear_results();

		// report warning
		std::ostream& warn();

		// report error
		std::ostream& err();
		
		// report critical error
		std::ostream& crit();
	private:
		compilation_results* _results;
		error_strbuf _buffer;
		std::ostream _stream;
	};
  } // namespace ink::compiler::internal
