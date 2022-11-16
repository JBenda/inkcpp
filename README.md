# inkcpp
![build](https://github.com/brwarner/inkcpp/workflows/build/badge.svg "Build Status")

Inkle Ink C++ Runtime with JSON -> Binary Compiler.

Ink Proofing Test Results: https://brwarner.github.io/inkcpp

## Project Goals
* Fast, simple, clean syntax
* No heap allocations during execution (unless in emergencies)
* No external dependencies, but extensions available for `Unreal` and `STL` by opt-in preprocessor defines
* Support for multiple "runners" (not ink threads) running in parallel on a single story that can optionally share a global variable/state store (but with their own callstack, temporaries, etc.)
* Multi-thread safe

## Example

```cpp
#include <ink/story.h>
#include <ink/runner.h>
#include <ink/choice.h>

using namespace ink::runtime;

int MyInkFunction(int a, int b) { return a + b; }

...

// Load ink binary story, generated from the inkCPP compiler
story* myInk = story::from_file("test.bin");

// Create a new thread
runner thread = myInk->new_runner();

// Register external functions (glue automatically generated via templates)
thread->bind("my_ink_function", &MyInkFunction);

// Write to cout
while(thread->can_continue())
	std::cout << thread->getline();

// Iterate choices
for(const choice& c : *thread) {
	std::cout << "* " << c.text() << std::endl;
}

// Pick the first choice
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

To build, either run the generated buildfiles OR you can use `cmake --build . --config <Release|Debug>` from the build folder to automatically execute the relevant toolchain.

For a more in depth installation description please checkout the (wiki)[https://github.com/brwarner/inkcpp/wiki/building].

## Including in C++ Code

Required software: (CMake)[https://cmake.org/]

Instructions:

1. Clone the repository
2. Configure and build the project with CMake, as described above
3. From your newly-created `build` directory, run `cmake --install . --prefix Path/To/Desired/Library/Directory --component <lib/cl/unreal>`.
4. Generated in your chosen directory, you will find a collection of folders. The following must be linked into your build solution for your C++ to compile correctly:
	- `include/ink`: contains important shared headers.
		+ For a Visual Studio project, link this directory as an Include Directory in VC++ Directories.
	- `lib/inkcpp.lib` and `lib/inkcpp_compiler.lib`: contains the library code for the InkCPP runner and compiler, respectively.
		+ For a Visual Studio project, link these files as Additional Dependencies in Linker->Input.
		+ You don't need to link the compiler if you're not using it within your program.
	- if you used the `cl` component you will find the `inkcpp_cl` executable in this location
	- for `unreal` you will find a `Source` directory containing the Unreal needed libs and headers. **Note:** not working for the current unreal version, we are working to fix this.
5. Reference the headers in your code like so:

```cpp

#include <ink/story.h>
#include <ink/runner.h>
#include <ink/choice.h>
```
6. if you use cmake checkout the (wiki)[https://github.com/brwarner/inkcpp/wiki/building#cmake-example] for including the library via cmake


### Troubleshooting

If you recieve an error like "Mismatch Detected for Runtime Library," it means you are probably using the Release version of the `.lib` files, but are running under a Debug configuration. To fix this, you can manually copy the `.lib` and `.pdb` files from `build/inkcpp/Debug` and/or `build/inkcpp_compiler/Debug` after running the build process again with `--config Debug` (see above). Then, you can add separate Debug and Release directories in the installed package folder, and change the paths based on your selected configuration in Visual Studio or otherwise, so that it links the Debug `.lib` for the Debug build, and the Release `.lib` for the Release build.


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

## Dependencies
The compiler depends on Nlohmann's JSON library and the C++ STL.

The runtime does not depend on either. If `INK_ENABLE_STL` is defined then STL extensions are added such as stream operators and `std::string` support. If `INK_ENABLE_UNREAL`, then FStrings, Delegates and other Unreal classes will be supported. 

NOTE: There is still some lingering C standard library calls in the runtime. I will be guarding them with an `INK_ENABLE_CSTD` or something soon.
