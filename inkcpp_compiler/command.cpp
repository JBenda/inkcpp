#include "command.h"

namespace ink
{
	// Command strings used by compiler
	const char* CommandStrings[] = {
			"STR",
			"INT",
			"FLOAT",
			"DIVERT_VAL",
			"\n",
			"<>",
			"void",
			"DIVERT",
			"DIVERT_TO_VARIABLE",
			"TUNNEL",
			"FUNCTION",
			"done",
			"end",
			"->->",
			"~ret",

			"DEFINE_TEMP",
			"SET_VARIABLE",

			"ev",
			"/ev",
			"out",
			"pop",
			"du",
			"PUSH_VARIABLE_VALUE",
			"visit",
			"READ_COUNT",
			"seq",
			"srnd",

			"str",
			"/str",

			"CHOICE",
			"thread",

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

			"START_CONTAINER",
			"END_CONTAINER",

			"CALL_EXTERNAL"
	};

	static_assert(sizeof(CommandStrings) / sizeof(const char*) == (int)Command::NUM_COMMANDS, "CommandStrings list much match Command enumeration");
}
