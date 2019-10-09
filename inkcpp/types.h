#pragma once

#include "story_ptr.h"

namespace ink::runtime
{
	class globals;
	class runner;

	typedef story_ptr<globals> globals_p;
	typedef story_ptr<runner> runner_p;
}