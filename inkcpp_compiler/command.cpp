#include "command.h"

namespace ink
{
	// Command strings used by compiler
	const char* CommandStrings[] = {
			"STR",
			"INT",
			"FLOAT",
			"DIVERT_VAL",
			"LIST",
			"\n",
			"<>",
			"void",
			"#",
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
			"rnd",
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
			"?",
			"!?",
			"L^",

			"!",
			"~",
			"LIST_COUNT",
			"LIST_MIN",
			"LIST_MAX",
			"lrnd",
			"LIST_ALL",
			"LIST_INVERT",

			"START_CONTAINER",
			"END_CONTAINER",

			"CALL_EXTERNAL"
	};

	template<unsigned A, unsigned B>
	struct equal {
		static_assert(A == B, "Not equal!");
	};
	equal<sizeof(CommandStrings) / sizeof(const char*), (int)(Command::NUM_COMMANDS)> dum;
	static_assert(sizeof(CommandStrings) / sizeof(const char*) == (int)Command::NUM_COMMANDS, "CommandStrings list muss match Command enumeration");
}
