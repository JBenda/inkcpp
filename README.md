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

int MyInkFunction(int a, int b) { return a + b; }

...

// Load ink binary story
story* myInk = story::from_file("test.bin");

// Create a new thread
runner thread = myInk->create_runner();

// Register external functions (glue automatically generated via templates)
thread->bind(ink::hash_string("my_ink_function"), &MyInkFunction);

// Write to cout
while(*thread)
	std::cout << *thread;

// Iterate choices
for(choice& c : *thread) {
	std::cout << "* " << c << std::endl;
}

// Pick a choice
thread->choose(0);

```

## Current Status
`inkcpp.cpp` has a `main()` function which compiles `test.json` -> `test.bin` and executes it using standard output and input.

Only very basic commands are supported right now.

* Temporary variables
* Int, String, or Divert values
* Eval stack (`ev`, `/ev`), String stack (`str`, `/str`)
* Choices (support for both `*` and `+` and conditional choices)
* Diverts (variable and fixed, conditional and normal)
* All mathematical operators (`+`, `<=`, etc.). String equality not supported.
* Glue
* Visit and read counts (`visits` and `CNT?` commands).
* `seq` command and all sequence types (stopping, cycle, shuffle)
* Global store that can be shared between runners
* External function binding (strings as arguments, but no string return values) (no fallback support yet)

## Next Steps

I have an Unreal plugin locally that integrates what I have so far. The next big step is to actually organize the files here into some coherant way so they can easily be packaged into either an Unreal plugin or just a bland, static library. Will probably use CMake or something.

After that, I really need to get dynamic string allocation working so the engine can use strings beyond what is in the compiled string table.

### Glaring Omissions

The big things we're missing right now are:

* Global variables
* Internal function calls. Fallback functions for externals.
* Threads
* Tunnels
* Choices whose text is not a single, fixed string
* Variable observers
* Lists and whatever cool, crazy stuff Ink has been adding recently.

Not to mention that the project is not organized to actually be used as a library or anything like that.

There are unit tests using `catch` for some of the underlying types (restorable stacks, arrays, and the shared-ish pointer) but not for any of the ink implementation.

## Dependencies
The compiler depends on Nlohmann's JSON library and the C++ STL.

The runtime does not depend on either. If `INK_ENABLE_STL` is defined then STL extensions are added such as stream operators and `std::string` support.
