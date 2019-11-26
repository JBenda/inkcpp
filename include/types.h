#pragma once

#include "story_ptr.h"

namespace ink
{

}

namespace ink::runtime
{
	class globals_interface;
	class runner_interface;

	typedef story_ptr<globals_interface> globals;
	typedef story_ptr<runner_interface> runner;
}