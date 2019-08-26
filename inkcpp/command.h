#pragma once

namespace binary
{
	// Commands (max 255)
	enum Command : unsigned char
	{
		COMMAND_START,
		STR = COMMAND_START,
		INT,
		DIVERT_VAL,
		DIVERT,
		DIVERT_TO_VARIABLE,
		DONE,
		END,

		DEFINE_TEMP,

		START_EVAL,
		END_EVAL,
		OUTPUT,

		START_STR,
		END_STR,

		CHOICE,

		BINARY_OPERATORS_START,

		ADD = BINARY_OPERATORS_START,
		IS_EQUAL,

		BINARY_OPERATORS_END,

		COMMAND_END = BINARY_OPERATORS_END,

		NUM_BINARY_OPERATORS = BINARY_OPERATORS_END - BINARY_OPERATORS_START
	};

	// Flags for commands
	enum CommandFlag : unsigned char
	{
		NO_FLAGS = 0,

		// == Choice Flags ==
		CHOICE_HAS_CONDITION = 1 << 0,
		CHOICE_HAS_START_CONTENT = 1 << 1,
		CHOICE_HAS_CHOICE_ONLY_CONTENT = 1 << 2,
		CHOICE_IS_INVISIBLE_DEFAULT = 1 << 3,
		CHOICE_IS_ONCE_ONLY = 1 << 4,
	};

#ifdef INK_COMPILER
	const char* CommandStrings[] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		"done",
		"end",

		nullptr,

		"ev",
		"/ev",
		"out",

		"str",
		"/str",

		nullptr,

		"+",
		"=="
	};

	static_assert(sizeof(CommandStrings) / sizeof(const char*) == COMMAND_END, "CommandStrings list much match Command enumeration");
#endif
}
