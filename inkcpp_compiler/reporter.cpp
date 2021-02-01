#include "reporter.h"
#include "compilation_results.h"
#include "system.h"

#include <iostream>
#include <sstream>

namespace ink::compiler::internal
{
	reporter::reporter()
		: _results(nullptr), _stream(&_buffer)
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
		// setp warning buffer
		if (_results != nullptr)
		{
			_buffer.start(&_results->warnings);
		}

		return _stream;
	}

	std::ostream& reporter::err()
	{
		// setp error buffer
		if (_results != nullptr)
		{
			_buffer.start(&_results->errors);
		}

		return _stream;
	}

	std::ostream& reporter::crit()
	{
		// setp error buffer
		if (_results != nullptr)
		{
			_buffer.start(&_results->errors);
			_buffer.throw_on_sync(true);
		}

		return _stream;
	}

	void error_strbuf::start(error_list* list)
	{
		// store list
		_list = list;

		// Make sure our buffer is empty
#ifdef WIN32
		_Tidy();
#endif
	}

	void error_strbuf::throw_on_sync(bool t)
	{
		_throw = t;
	}

	int error_strbuf::sync()
	{
		// TODO: Assert?
		if (_list == nullptr)
			return -1;

		// Add string to list
		std::string val = str();
		_list->push_back(val);

		// Clear our state
		_list = nullptr;
#ifdef WIN32
		_Tidy();
#endif

		// Should we throw?
		if (_throw)
		{
			_throw = false;
			throw ink::ink_exception(("CRITICAL ERROR: " + val).c_str());
		}

		// Return success
		return 0;
	}
}
