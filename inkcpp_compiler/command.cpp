#include "command.h"

namespace ink
{
	// Command strings used by compiler
	const char* CommandStrings[] = {
			"inkcpp_STR",
			"inkcpp_INT",
			"inkcpp_BOOL",
			"inkcpp_FLOAT",
			"inkcpp_VALUE_POINTER",
			"inkcpp_DIVERT_VAL",
			"inkcpp_LIST",
			"\n",
			"<>",
			"void",
			"inkcpp_TAG",
			"inkcpp_DIVERT",
			"inkcpp_DIVERT_TO_VARIABLE",
			"inkcpp_TUNNEL",
			"inkcpp_FUNCTION",
			"done",
			"end",
			"->->",
			"~ret",

			"inkcpp_DEFINE_TEMP",
			"inkcpp_SET_VARIABLE",

			"ev",
			"/ev",
			"out",
			"pop",
			"du",
			"inkcpp_PUSH_VARIABLE_VALUE",
			"visit",
			"turn",
			"inkcpp_READ_COUNT",
			"seq",
			"srnd",

			"str",
			"/str",
			"#",
			"/#",

			"inkcpp_CHOICE",
			"thread",

			"range",

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
			"listInt",

			"!",
			"~",
			"LIST_COUNT",
			"LIST_MIN",
			"LIST_MAX",
			"readc",
			"turns",
			"lrnd",
			"FLOOR",
			"CEILING",
			"INT",
			"LIST_ALL",
			"LIST_INVERT",
			"LIST_VALUE",
			"choiceCnt",

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
