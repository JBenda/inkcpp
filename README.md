# inkcpp
Inkle Ink C++ Runtime with JSON -> Binary Compiler.

## Project Goals
* Fast, simple, clean syntax
* No heap allocations during execution (unless in emergencies)
* No external dependencies, but extensions available for `Unreal` and `STL` by opt-in preprocessor defines
* Support for multiple "runners" (not ink threads) running in parallel on a single story that can optionally share a global variable/state store (but with their own callstack, temporaries, etc.)
* Multi-thread safe

## Example

```cpp
using namespace ink::runtime;

// Load ink binary story
story myInk("story.bin");

// Create a new thread at the start of the story
runner thread(&myInk);

// Write to cout
while(thread)
	std::cout << thread;

// Iterate choices
for(choice& c : thread) {
	std::cout << "* " << c.text() << std::endl;
}

// Pick a choice
thread.choose(0);

```

## Current Status
`inkcpp.cpp` has a `main()` function which compiles `test.json` -> `test.bin` and executes it using standard output and input.

Only very basic commands are supported right now.

* Temporary variables
* Int, String, or Divert values
* Eval stack (`ev`, `/ev`), String stack (`str`, `/str`)
* Basic choices (no differentiation between `*` and `+`, no conditions)
* Diverts (variable and fixed, conditional and normal)
* All mathematical operators (`+`, `<=`, etc.). String equality not supported.
* Glue

## Dependencies
The compiler depends on Nlohmann's JSON library and the C++ STL.

The runtime does not depend on either. If `INK_ENABLE_STL` is defined then STL extensions are added such as stream operators and `std::string` support.
