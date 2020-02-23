#pragma once

namespace ink
{
	// Commands (max 255)
	enum class Command : unsigned char
	{
		// == Value Commands: Push values onto the stack
		STR,
		INT,
		DIVERT_VAL,
		NEWLINE,
		GLUE,

		// == Diverts
		DIVERT,
		DIVERT_TO_VARIABLE,

		// == Terminal commands
		DONE,
		END,

		// == Variable definitions
		DEFINE_TEMP,
		SET_VARIABLE,

		// == Evaluation stack
		START_EVAL,
		END_EVAL,
		OUTPUT,
		POP,
		DUPLICATE,
		PUSH_VARIABLE_VALUE,
		VISIT,
		READ_COUNT,
		SEQUENCE,

		// == String stack
		START_STR,
		END_STR,

		// == Choice commands
		CHOICE,

		// == Binary operators
		BINARY_OPERATORS_START,
		ADD = BINARY_OPERATORS_START,
		SUBTRACT,
		DIVIDE,
		MULTIPLY,
		MOD,
		IS_EQUAL,
		GREATER_THAN,
		LESS_THAN,
		GREATER_THAN_EQUALS,
		LESS_THAN_EQUALS,
		NOT_EQUAL,
		AND,
		OR,
		MIN,
		MAX,
		BINARY_OPERATORS_END = MAX,

		// == Unary operators
		UNARY_OPERATORS_START,
		NOT = UNARY_OPERATORS_START,
		NEGATE,
		UNARY_OPERATORS_END = NEGATE,

		// == Container tracking
		START_CONTAINER_MARKER,
		END_CONTAINER_MARKER,

		// == Function calls
		CALL_EXTERNAL,

		NUM_COMMANDS,
	};

	// Flags for commands
	enum class CommandFlag : unsigned char
	{
		NO_FLAGS = 0,

		// == Choice Flags ==
		CHOICE_HAS_CONDITION = 1 << 0,
		CHOICE_HAS_START_CONTENT = 1 << 1,
		CHOICE_HAS_CHOICE_ONLY_CONTENT = 1 << 2,
		CHOICE_IS_INVISIBLE_DEFAULT = 1 << 3,
		CHOICE_IS_ONCE_ONLY = 1 << 4,

		// == Divert flags
		DIVERT_HAS_CONDITION = 1 << 0,
		DIVERT_IS_FALLTHROUGH = 1 << 1, // Divert was auto-generated as a result of falling out of a containers content

		// == Container marker
		CONTAINER_MARKER_TRACK_VISITS = 1 << 0,
		CONTAINER_MARKER_TRACK_TURNS = 1 << 1,

		// == Variable assignment
		ASSIGNMENT_IS_REDEFINE = 1 << 0,
	};

	inline bool operator& (CommandFlag lhs, CommandFlag rhs)
	{
		return (static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs)) > 0;
	}

	inline CommandFlag& operator|= (CommandFlag& lhs, CommandFlag rhs)
	{
		lhs = static_cast<CommandFlag>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
		return lhs;
	}

#ifdef INK_COMPILER
	const char* CommandStrings[] = {
		nullptr,
		nullptr,
		nullptr,
		"\n",
		"<>",
		nullptr,
		nullptr,
		"done",
		"end",

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
#endif
}
