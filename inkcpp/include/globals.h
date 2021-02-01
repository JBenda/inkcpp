#pragma once

#include "system.h"

namespace ink::runtime
{
	/**
	* Represents a global store to be shared amongst ink runners.
	* Stores global variable values, visit counts, turn counts, etc.
	*/
	class globals_interface
	{
	public:
		// No public interface yet
		virtual void dummy() = 0;
		virtual ~globals_interface() = default;
	};
}
