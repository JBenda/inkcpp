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
thread->bind("my_ink_function", &MyInkFunction);

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
Run `inkcpp_cl.exe -p myfile.json` to execute a compiled Ink JSON file in play mode. It can also operate on `.ink` files but `inklecate.exe` must me in the same folder or in the PATH.

Without the `-p` flag, it'll just compile the JSON/Ink file into InkCPP's binary format (see the Wiki on GitHub).

Many, but not all features of the Ink language are supported (see Glaring Omissions below), but be warned, this runtime is still highly unstable. I am currently working on getting it to pass all the unit tests on [ink-proof](https://github.com/chromy/ink-proof).

* Temporary and global variables
* Int, String, or Divert values
* Eval stack (`ev`, `/ev`), String stack (`str`, `/str`)
* Choices (support for both `*` and `+` and conditional choices)
* Diverts (variable and fixed, conditional and normal)
* All mathematical operators (`+`, `<=`, etc.). String equality not supported.
* Glue
* Visit and read counts (`visits` and `CNT?` commands).
* `seq` command and all sequence types (stopping, cycle, shuffle)
* Global store that can be shared between runners
* External function binding (no fallback support yet)
* Tunnels and internal functions
* Ink threads (probably incredibly unstable though)

## Configuring and Building (CMake)

To configure the project...

1. Install CMake
2. Create a folder called `build`
3. From the build folder, run `cmake ..`

CMake will then generate the necessary build files for your environment. By default, it generates Visual Studio projects and solutions on Windows and Makefiles on Mac and Linux. You can change this using CMake's command line options (see `cmake --help`). It supports pretty much anything.

To build, either run the generated buildfiles OR you can use `cmake --build` from the build folder to automatically execute the relevant toolchain.

### Running Tests

Run `ctest` from the build folder to execute unit tests configured with CMake. Use `ctest -V` for more verbose error output.

Right now this only executes the internal unit tests which test the functions of particular classes. Soon it'll run more complex tests on .ink files using ink-proof.

## Unreal Plugin

Code for the Unreal plugin is located in the `unreal` directory. In order to install it, run `cmake --install . --component unreal --prefix Path/To/Unreal/Plugins/` which will add an `inkcpp` folder there with the `.uplugin`, the code for the UClasses, and all the inkcpp source files required. `config.h` will automatically detect it is being built in an Unreal plugin environment and disable STL and enable Unreal extensions (FString support, Unreal asserts, CityHash, etc.).

## Next Steps

I am currently working toward a 1.0 release. You can track my progress here: https://github.com/brwarner/inkcpp/projects/1

### Glaring Omissions

The big things we're missing right now are:

* Fallback functions for externals.
* Variable observers
* Lists and whatever cool, crazy stuff Ink has been adding recently.
* Robust tests using ink-proof.

## Dependencies
The compiler depends on Nlohmann's JSON library and the C++ STL.

The runtime does not depend on either. If `INK_ENABLE_STL` is defined then STL extensions are added such as stream operators and `std::string` support. If `INK_ENABLE_UNREAL`, then FStrings, Delegates and other Unreal classes will be supported. 

NOTE: There is still some lingering C standard library calls in the runtime. I will be guarding them with an `INK_ENABLE_CSTD` or something soon.
