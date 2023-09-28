#pragma once

namespace ink
{
	// Commands (max 255)
	enum class Command : unsigned char
	{
		// == Value Commands: Push values onto the stack
		STR,
		INT,
		BOOL,
		FLOAT,
		VALUE_POINTER,
		DIVERT_VAL,
		LIST,
		NEWLINE,
		GLUE,
		VOID,
		TAG,

		// == Diverts
		DIVERT,
		DIVERT_TO_VARIABLE,
		TUNNEL,
		FUNCTION,

		// == Terminal commands
		DONE,
		END,
		TUNNEL_RETURN,
		FUNCTION_RETURN,

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
		TURN, /// How many choices where made since start of the story
		READ_COUNT,
		SEQUENCE,
		SEED,

		// == String stack
		START_STR,
		END_STR,
		START_TAG,
		END_TAG,

		// == Choice commands
		CHOICE,

		// == Threading
		THREAD,
		// == thinary operations
		LIST_RANGE,
		OP_BEGIN = LIST_RANGE,
		TERNARY_OPERATORS_START = LIST_RANGE,
		TERNARY_OPERATORS_END = LIST_RANGE,
		// == Binary operators
		ADD,
		BINARY_OPERATORS_START = ADD,
		SUBTRACT,
		DIVIDE,
		MULTIPLY,
		MOD,
		RANDOM,
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
		HAS,
		HASNT,
		INTERSECTION,
		LIST_INT,
		BINARY_OPERATORS_END = LIST_INT,

		// == Unary operators
		UNARY_OPERATORS_START,
		NOT = UNARY_OPERATORS_START,
		NEGATE,
		LIST_COUNT,
		LIST_MIN,
		LIST_MAX,
		READ_COUNT_VAR,
		TURNS,
		lrnd,
		FLOOR,
		CEILING,
		INT_CAST,
		LIST_ALL,
		LIST_INVERT,
		LIST_VALUE,
		UNARY_OPERATORS_END = LIST_VALUE,
		CHOICE_COUNT,
		OP_END,

		// == Container tracking
		START_CONTAINER_MARKER = OP_END,
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
		CONTAINER_MARKER_ONLY_FIRST = 1 << 2,

		// == Variable assignment
		ASSIGNMENT_IS_REDEFINE = 1 << 0,

		// == Function/Tunnel flags
		FUNCTION_TO_VARIABLE = 1 << 0,
		TUNNEL_TO_VARIABLE = 1 << 0,
		FALLBACK_FUNCTION = 1 << 1,
		// note a internal function which should only be called if external reference is not working
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

	template<typename PayloadType>
	constexpr unsigned int CommandSize = sizeof(Command) + sizeof(CommandFlag) + sizeof(PayloadType);

#ifdef INK_COMPILER
	extern const char* CommandStrings[];
#endif
}
