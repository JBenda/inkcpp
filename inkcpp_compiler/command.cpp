#include "command.h"

namespace ink
{
	// Command strings used by compiler
	const char* CommandStrings[] = {
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			"\n",
			"<>",
			"void",
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			"done",
			"end",
			"->->",
			"~ret",

			nullptr,
			nullptr,

			"ev",
			"/ev",
			"out",
			"pop",
			"du",
			nullptr,
			"visit",
			nullptr,
			"seq",
			"srnd",

			"str",
			"/str",

			nullptr,

			"+",
			"-",
			"/",
			"*",
			"%",
			"==",
			">",
			"<",
			">=",
			"<=",
			"!=",
			"&&",
			"||",
			"MIN",
			"MAX",

			"!",
			"~",

			nullptr,

			nullptr,
			nullptr
	};

	static_assert(sizeof(CommandStrings) / sizeof(const char*) == (int)Command::NUM_COMMANDS, "CommandStrings list much match Command enumeration");
}