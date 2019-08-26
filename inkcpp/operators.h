#pragma once

#include "value.h"
#include "command.h"

namespace binary 
{
	namespace runtime
	{
		// Runs a binary operator on two values and returns the result
		value evaluate(Command command, const value& left, const value& right);
	}
}