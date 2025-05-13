/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include <iostream>
#include <cstdint>

namespace ink
{
// Commands (max 255)
enum class Command : uint8_t {
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
	OP_BEGIN                = LIST_RANGE,
	TERNARY_OPERATORS_START = LIST_RANGE,
	TERNARY_OPERATORS_END   = LIST_RANGE,
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
	INVERT,
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

extern const char* CommandStrings[];

inline std::ostream& operator<<(std::ostream& out, Command cmd)
{
	using number = std::underlying_type_t<Command>;
	out << CommandStrings[static_cast<number>(cmd)];

	return out;
}

// Flags for commands
enum class CommandFlag : uint8_t {
	NO_FLAGS = 0,

	// == Choice Flags ==
	CHOICE_HAS_CONDITION           = 1 << 0,
	CHOICE_HAS_START_CONTENT       = 1 << 1,
	CHOICE_HAS_CHOICE_ONLY_CONTENT = 1 << 2,
	CHOICE_IS_INVISIBLE_DEFAULT    = 1 << 3,
	CHOICE_IS_ONCE_ONLY            = 1 << 4,

	// == Divert flags
	DIVERT_HAS_CONDITION = 1 << 0,
	DIVERT_IS_FALLTHROUGH
	    = 1 << 1, // Divert was auto-generated as a result of falling out of a containers content

	// == Container marker
	CONTAINER_MARKER_TRACK_VISITS = 1 << 0,
	CONTAINER_MARKER_TRACK_TURNS  = 1 << 1,
	CONTAINER_MARKER_ONLY_FIRST   = 1 << 2,
	CONTAINER_MARKER_IS_KNOT      = 1 << 3,

	// == Variable assignment
	ASSIGNMENT_IS_REDEFINE = 1 << 0,

	// == Function/Tunnel flags
	FUNCTION_TO_VARIABLE = 1 << 0,
	TUNNEL_TO_VARIABLE   = 1 << 0,
	FALLBACK_FUNCTION    = 1 << 1,
	// note a internal function which should only be called if external reference is not working
};

inline std::ostream& operator<<(std::ostream& out, CommandFlag cmd)
{
	using number = std::underlying_type_t<CommandFlag>;

	if (cmd == CommandFlag::NO_FLAGS) {
		return out << "NO_FLAGS";
	}

	const size_t value = static_cast<number>(cmd);
	size_t       count = 0;

#define CHECK_FLAG(_NAME)                                       \
	if ((value & static_cast<number>(CommandFlag::_NAME)) != 0) { \
		if (count++ > 0) {                                          \
			out << " | ";                                             \
		}                                                           \
		out << #_NAME;                                              \
	}

	CHECK_FLAG(CHOICE_HAS_CONDITION);
	CHECK_FLAG(CHOICE_HAS_START_CONTENT);
	CHECK_FLAG(CHOICE_HAS_CHOICE_ONLY_CONTENT);
	CHECK_FLAG(CHOICE_IS_INVISIBLE_DEFAULT);
	CHECK_FLAG(CHOICE_IS_ONCE_ONLY);
	CHECK_FLAG(DIVERT_HAS_CONDITION);
	CHECK_FLAG(DIVERT_IS_FALLTHROUGH);
	CHECK_FLAG(CONTAINER_MARKER_TRACK_VISITS);
	CHECK_FLAG(CONTAINER_MARKER_TRACK_TURNS);
	CHECK_FLAG(CONTAINER_MARKER_ONLY_FIRST);
	CHECK_FLAG(ASSIGNMENT_IS_REDEFINE);
	CHECK_FLAG(FUNCTION_TO_VARIABLE);
	CHECK_FLAG(TUNNEL_TO_VARIABLE);
	CHECK_FLAG(FALLBACK_FUNCTION);

#undef CHECK_FLAG

	return out;
}

inline bool operator&(CommandFlag lhs, CommandFlag rhs)
{
	using number = std::underlying_type_t<CommandFlag>;
	return (static_cast<number>(lhs) & static_cast<number>(rhs)) > 0;
}

inline CommandFlag& operator|=(CommandFlag& lhs, CommandFlag rhs)
{
	using number = std::underlying_type_t<CommandFlag>;
	lhs          = static_cast<CommandFlag>(static_cast<number>(lhs) | static_cast<number>(rhs));
	return lhs;
}

template<typename PayloadType>
constexpr unsigned int CommandSize = sizeof(Command) + sizeof(CommandFlag) + sizeof(PayloadType);

} // namespace ink
