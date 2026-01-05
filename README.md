# inkcpp
![build](https://github.com/JBenda/inkcpp/workflows/build/badge.svg "Build Status")

Inkle Ink C++ Runtime with JSON -> Binary Compiler.

Ink Proofing Test Results: https://jbenda.github.io/inkcpp/proof

Doxygen Documentation: https://jbenda.github.io/inkcpp/html

CLib Documentation: https://jbenda.github.io/inkcpp/html/group__clib.html

UE Documentation: https://jbenda.github.io/inkcpp/html/group__unreal.html

Python Documentation: https://jbenda.github.io/inkcpp/html/inkcpp_py.html

## Project Goals
* Fast, simple, clean syntax
* No heap allocations during execution (unless in emergencies)
* No external dependencies, but extensions available for `Unreal` and `STL` by opt-in preprocessor defines
* Support for multiple "runners" (not ink threads) running in parallel on a single story that can optionally share a global variable/state store (but with their own callstack, temporaries, etc.)
* Multi-thread safe


## Current Status

supported languages ([latest release](https://github.com/JBenda/inkcpp/releases/latest)):
	+ C++ [doc](https://jbenda.github.io/inkcpp/html/index.html)[example](https://jbenda.github.io/inkcpp/html/index.html)
	+ C [doc](https://jbenda.github.io/inkcpp/html/group__clib.html)[example](https://jbenda.github.io/inkcpp/html/group__clib.html#example_c)
	+ UE Blueprints [doc](https://jbenda.github.io/inkcpp/html/group__unreal.html)[distribution](https://www.unrealengine.com/marketplace/product/inkcpp)[example](https://jbenda.github.io/inkcpp/html/group__unreal.html#ue_example)
	+ Python [doc](https://jbenda.github.io/inkcpp/html/inkcpp_py.html)[distribution](https://pypi.org/project/inkcpp-py/)[example](https://jbenda.github.io/inkcpp/html/index.html#py)

Run `inkcpp_cl.exe -p myfile.json` to execute a compiled Ink JSON file in play mode. It can also operate on `.ink` files but `inklecate.exe` must be in the same folder or in the PATH.
`inklecate` can be downloaded from the [official release page](https://github.com/inkle/ink/releases) and will be downloaded from CMake at  configure time (located at `build/unreal/inkcpp/Resources/inklecate`).
Or do it automatically with the `INKCPP_INKLECATE=OS` CMake flag. (It will be downloaded to `<build-dir>/inklecate/<os>/` and will be installed with `cmake --install . --component cl`)

Without the `-p` flag, it'll just compile the JSON/Ink file into InkCPP's binary format (see the Wiki on GitHub).

All features of ink 1.1 are supported, and checked with [ink-proof](https://github.com/chromy/ink-proof).

In addition a UE Plugin inclusive BluePrints are provided and python bindings based on [pybind11](https://github.com/pybind/pybind11).

KeyFeatures: snapshots, observers, binding ink functions, support ink [function fallback](https://github.com/inkle/ink/blob/master/Documentation/RunningYourInk.md#fallbacks-for-external-functions)

## Unreal Plugin

InkCPP is available via the [UE Marketplace](https://www.unrealengine.com/marketplace/product/inkcpp).

Alternativly is the latest version of the UE plugin can be downloaded from the [release page](https://github.com/JBenda/inkcpp/releases/latest) (`unreal.zip`).
Place the content of this file at your plugin folder of your UE project and at the next start up it will be intigrated.


A example project can be found [here](https://jbenda.github.io/inkcpp/unreal/InkCPP_DEMO.zip). And here the [Documentation](https://jbenda.github.io/inkcpp/html/group__unreal.html).

Code for the Unreal plugin is located in the `unreal` directory. In order to install it, run
```sh
mkdir build
cd build
mkdir plugin
mkdir plugin-build
cmake -DINKCPP_UNREAL_TARGET_VERSION="5.5" ..
cmake --install . --component unreal --prefix .\plugin  # create source files for plugin
\PATH\TO\UNREAL_ENGINE\Build\BatchFiles\RunUAT.bat BuildPlugin -plugin=GIT_REPO\build\plugin\inkcpp\inkcpp.uplugin -package=GIT_REPO\build\plugin-build\inkcpp -TargetPlatforms=Win64 # compile plugin
move plugin-build\inkcpp UE_ENGINE\Engine\Plugins\inkcpp
```
Adapt `TargetPlatforms` as nessesarry. You might also want to install the Plugin directly into a project or the in UE5.5 introduced external plugin directory. Just adapt the pathets accordendly.

## Use standalone

1. Grep the current version from the [release page](https://github.com/JBenda/inkcpp/releases/latest) depending on your OS (e.g. macos-cl).
2. unpack it to a location found by your path
3. run your story: `inkcpp-cl -p story.json`
4. if you want to compile `.ink` flies directly make sure `inklecate` is in your path. If you not have it you can grep it from the [official page](https://github.com/inkle/ink/releases/latest)

Nice features for testing:
+ predefined choice selection `echo 1 2 1  | inkpp-cl -p story.(ink|json|bin)`
+ create snapshots to shorten testing:
	+ create snapshot by entering `-1` as choice `echo 1 2 -1 | inkcpp-cl -p story.ink`
 	+ load snapshot as an additional argument `echo 1 | inkcpp-cl -p story.snap story.ink`

## Including in C++ Code

Instructions:

1. Download the for your OS macthing lib archive from the [release page](https://github.com/JBenda/inkcpp/releases/latest) (e.g. linux-lib).
2. The following must be linked into your build solution for your C++ to compile correctly:
	- `include/ink`: contains important shared headers.
		+ For a Visual Studio project, link this directory as an Include Directory in VC++ Directories.
	- `lib/inkcpp.lib` and `lib/inkcpp_compiler.lib`: contains the library code for the InkCPP runner and compiler, respectively.
		+ For a Visual Studio project, link these files as Additional Dependencies in Linker->Input.
		+ You don't need to link the compiler if you're not using it within your program.
5. Reference the headers in your code like so:
```cpp

#include <ink/story.h>
#include <ink/runner.h>
#include <ink/choice.h>
```
6. if you use cmake checkout the (wiki)[https://github.com/JBenda/inkcpp/wiki/building#cmake-example] for including the library via cmake


### Example

```cpp
#include <ink/story.h>
#include <ink/runner.h>
#include <ink/choice.h>
#include <memory.h>

using namespace ink::runtime;

int MyInkFunction(int a, int b) { return a + b; }

...

// Load ink binary story, generated from the inkCPP compiler
std::unique_ptr<story> myInk{story::from_file("test.bin")};

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


## Configuring and Building (CMake)

To configure the project...

1. Install CMake
2. Create a folder called `build`
3. From the build folder, run `cmake ..`

CMake will then generate the necessary build files for your environment. By default, it generates Visual Studio projects and solutions on Windows and Makefiles on Mac and Linux. You can change this using CMake's command line options (see `cmake --help`). It supports pretty much anything.

The documentation can be build iff Doxygen is installed with `cmake --build . --target doc`. The documentation can then be found in at `html/index.html`.

To build, either run the generated buildfiles OR you can use `cmake --build . --config <Release|Debug>` from the build folder to automatically execute the relevant toolchain.

To install the different components use `cmake --install . --component <lib|cl|unreal|unreal_plugin>`
+ `lib` C++ library to link against
+ `clib` C library to link against
+ `cl` command line application
+ `unreal` UE-plugin Source
+ `unreal_plugin` UE-plugin compiled, requires to call `cmake --build . --target unreal` before!

For a more in depth installation description please checkout the [wiki](https://github.com/JBenda/inkcpp/wiki/building).


### Troubleshooting

If you recieve an error like "Mismatch Detected for Runtime Library," it means you are probably using the Release version of the `.lib` files, but are running under a Debug configuration. To fix this, you can manually copy the `.lib` and `.pdb` files from `build/inkcpp/Debug` and/or `build/inkcpp_compiler/Debug` after running the build process again with `--config Debug` (see above). Then, you can add separate Debug and Release directories in the installed package folder, and change the paths based on your selected configuration in Visual Studio or otherwise, so that it links the Debug `.lib` for the Debug build, and the Release `.lib` for the Release build.


### Running Tests

To enable testing set the CMake flag `INKCPP_TEST=ON`. If you do not have inklecate at your path you can set `INKCPP_INKLECATE=OS` to download und use the current supported verision.
Run `ctest -C Release` from the build folder to execute unit tests configured with CMake. Use `ctest -V -Release` for more verbose error output.
Do not forgett that the C libs are only testet if `INKCPP_C=ON` is set.

```sh
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DINKCPP_TEST=ON -DINKCPP_INKLECATE=OS
cmake --build . --config Release
ctest -C Release
```

To test the python bindings use:

```sh
python -m pip install build pytest
python -m build
python -m pip install dist/*.whl --user
# if inklecate is not in the same directory / inside Path set INKLECATE enviroment variable
export INKLECATE=$PWD/build/inklecate/linux/inklecate # linux
set INKLECTATE=%CD%/build/inklecate/windows/inklecate.exe   # windows
python -m pytest inkcpp_python/tests
```

Right now this only executes the internal unit tests which test the functions of particular classes. Soon it'll run more complex tests on .ink files using ink-proof.


## Python Bindings

The easy way to start is installing it with pip: `pip install inkcpp_py`.
An example can be found at [example.py](./inkcpp_python/example.py).
To build it from source use:

```sh
git clone --recurse-submodules https://github.com/JBenda/inkcpp.git
pip install .
```

The python bindnigs are defined in `inkcpp_python` subfolder.

## Dependencies
The compiler depends on Nlohmann's JSON library and the C++ STL.

The runtime does not depend on either. If `INK_ENABLE_STL` is defined then STL extensions are added such as stream operators and `std::string` support. If `INK_ENABLE_UNREAL`, then FStrings, Delegates and other Unreal classes will be supported.

NOTE: There is still some lingering C standard library calls in the runtime. I will be guarding them with an `INK_ENABLE_CSTD` or something soon.
