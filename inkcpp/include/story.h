#pragma once

#include "types.h"

namespace ink::runtime
{
/**
 * A loaded ink story.
 *
 * Created by loading a binary ink story into memory. Once loaded,
 * the story class can create "runners" which execute story code.
 * A story can have any number of runners, which can optionally
 * share globals (variables, visit counts, etc). through the
 * globals object. By default, each runner gets its own newly
 * created globals store.
 * @see ink::runtime::runner_interface
 * @see ink::runtime::globals_interface
 */
class story
{
public:
	virtual ~story(){};
#pragma region Interface Methods
		/**
		 * Creates a new global store
		 *
		 * Creates a new global store that can be passed in when
		 * creating new runners for this story. Note: Can not be
		 * used for other stories. It is tied to this story.
		 *
		 * @return managed pointer to a new global store
		*/
		virtual globals new_globals() = 0;
	  /** Reconstructs globals from snapshot
	   * @param obj snapshot to load
	   */
	  virtual globals new_globals_from_snapshot(const snapshot& obj) = 0;

	  /**
	   * Creates a new runner
	   *
	   * Creates a new runner whose initial instruction pointer
	   * is the first instruction in this story. If no global
	   * store is passed, a new one will be created for the runner.
	   *
	   * @param store globals to use for the runner
	   * @return managed pointer to a new runner
	   */
	  virtual runner new_runner(globals store = nullptr) = 0;
	  /**
	   * @brief reconstruct runner from a snapshot
	   * @attention runner must be snap_shotted from the same story
	   * @attention if globals is explicit set,
	   * make sure the globals are from the same snapshot as
	   * @attention if you snap_shotted a multiple runner with shared global
	   * please reconstruct it in the same fashion
	   * @param obj
	   * @param store can be set if explicit access to globals is required or multiple runner with a
	   * shared global are used
	   * @param runner_id if the snapshot was of a multiple runner one global situation load first the
	   * global, and then each runner with global set and increasing idx
	   */
	  virtual runner new_runner_from_snapshot(
	      const snapshot& obj, globals store = nullptr, unsigned runner_id = 0
	  ) = 0;
#pragma endregion

#pragma region Factory Methods
		/**
		 * Creates a new story object from a file.
		 *
		 * Requires STL or other extension which allows files
		 * to be loaded and read. Will allocate all the data
		 * necessary to load the file and close it.
		 *
		 * @param filename filename of the binary ink data
		 * @return new story object
		*/
		static story* from_file(const char* filename);

		/**
		 * Create a new story object from binary buffer
		 *
		 * No extensions required. Creates the story from binary
		 * data already loaded into memory. By default, the story
		 * will free this buffer when it is destroyed.
		 *
		 * @param data binary data
		 * @param length of the binary data in bytes
		 * @param freeOnDestroy if true, free this buffer once the story is destroyed
		 * @return new story object
		*/
		static story* from_binary(unsigned char* data, size_t length, bool freeOnDestroy = true);
#pragma endregion
};
}

/** @namespace ink
 * Namespace contaning all modules and classes from InkCPP
 *
 * (Unreal Blueprint Classes Excluded, but there will not be there in a normal build)
 */

/** @namespace ink::runtime
 * Contaning all modules and classes used for the inkles ink runtime.
 * A minimal example can be found at @ref src_main
 */

/** @mainpage InkCPP Documentation
 * @tableofcontents
 * Inkle Ink C++ Runtime with INK.JSON -> Binary Compiler.<br/>
 * supports ussage in:
 * + C++ (with CMAKE)
 * + UE
 * + Python [inkcpp_py](https://pypi.org/project/inkcpp-py/)
 *
 * @section cmake CMAKE usage
 *
 * The current erlease is available at the [release
 * page](https://github.com/JBenda/inkcpp/releases/latest), as `<os>-lib.zip` (e.g.
 * `linux-lib.zip`). <br/> to link the libraries you can use `find_package(inkcpp CONFIG)` which
 * provides two targets:
 * + inkcpp: the runtime enviroment
 * + inkcpp_comopiler: functionality to compile a story.json to story.bin
 *
 * To run your own `.ink` files you need a way to compile it to inks runtime format `.ink.json`. One
 * way is to use `inklecate <story>.ink`.<br/> Which is available at the [official release
 * page](https://github.com/inkle/ink/releases/latest).<br/>
 *
 * If you want to use the inkcpp with C link against the target inkcpp_c and `#include
 * <ink/c/inkcpp.h>` The C-API documentation and example can be found @ref clib "here".
 *
 * Exampl with library extracted at /YOUR/PROJECT/linux-lib
 * And the [Example project](../cmake_example.zip) is extracted to /YOUR/PROJECT
 * @code {sh}
 * cd /YOUR/PROJECT
 * ls # expected output: CMakeLists.txt main.cpp test.ink test.ink.json linux-lib
 * mkdir build
 * cd build
 * inkcpp_DIR=../linux-lib cmake ..
 * cmake --build .
 * cp ../test.ink.json .
 * ./main_cpp
 * @endcode
 *
 * @subsection src_main main.cpp
 * @include cmake_example/main.cpp
 *
 * @subsection src_cmake CMakeLists.txt
 * @include cmake_example/CMakeLists.txt
 *
 * @subsection src_story_json test.ink
 * @include cmake_example/test.ink
 * compiled: [test.ink.json](../cmake_example/test.ink.json)
 *
 * @section ue Unreal Installation
 *
 * The current release is available at the [release
 * page](https://github.com/JBenda/inkcpp/releases/latest), as `unreal.zip`.<br/>
 * Unpack this foldor in `/PATH/TO/UNREAL_ENGINE/Engine/Plugins/` and it will be available
 * as plugin in the plugin list. <br/>
 * Or unpack this folder in `/PATH/TO/UNREAL_PROJECT/Plugins/` and it will be
 * intigrated at the next startup.<br/> A MarketPlace appearance is work in progress :)
 *
 * The overview to the UE Blueprint class and examples can be found at @ref unreal "here".
 *
 * If you want to use the newest version clone the project and install the unreal component.
 * @code {sh}
 * git clone https://github.com/JBenda/inkcpp
 * cd inkcpp
 * mkdir build
 * mkdir plugin
 * cd build
 * cmake ..
 * cmake --install . --component unreal --prefix ../plugin
 * cd ../plugin
 * # Should contain a folder named 'inkcpp'
 * cp -r inkcpp /PATH/TO/UNREAL_PROJECT/Plugins
 * @endcode
 *
 * @section py Python example
 *
 * You can install the current release from [pypi](https://pypi.org/project/inkcpp-py/) with <br/>
 * `pip install inkcpp-py`.<br/>
 * Or build it yourself from main with: <br/>
 * `pip install .`
 *
 * Here can you find an
 * [example](https://raw.githubusercontent.com/JBenda/inkcpp/master/inkcpp_py/example.py) inclusive
 * [story](https://raw.githubusercontent.com/JBenda/inkcpp/master/inkcpp_py/unreal_example.ink).
 *
 * [Python module documentation](./inkcpp_py.html)
 */
