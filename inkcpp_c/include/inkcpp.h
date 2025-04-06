#ifndef _INKCPP_H
#define _INKCPP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#else
typedef struct HInkSnapshot HInkSnapshot;
typedef struct HInkChoice   HInkChoice;
typedef struct HInkList     HInkList;
typedef struct InkListIter  InkListIter;
typedef struct InkFlag      InkFlag;
typedef struct InkValue     InkValue;
typedef struct HInkRunner   HInkRunner;
typedef struct HInkGlobals  HInkGlobals;
typedef struct HInkSTory    HInkStory;
#endif
	typedef uint32_t ink_hash_t;

	/** @defgroup clib Clib Interface
	 * C bindings for inkcpp
	 *
	 * There are two different ways to get the C bindings.
	 * 1. Use the distributed `<os>-lib.zip` for a C++ and C lib combined, then use CMake for the
	 * linkadge (or do it manually)
	 * 2. Use the distrubuted `<os>-clib.zip` for a C only lib, then use CMake, or pkg-config.
	 *
	 * Please note that the included header is different between this two installation methods.
	 * 1. `#include <ink/c/inkcpp.h>`
	 * 2. `#include <ink/inkcpp.h>`
	 *
	 * @section example_c Example
	 *
	 * To setup an example for option `1.` and `2.` if you use cmake checkout @ref cmake and replace
	 * `target_link_libraries` with `target_link_libraries(main inkcpp_c)` The story and source file
	 * can be used as @ref src_main_c "noted down"
	 *
	 * For setup an example for option `2.` without cmake create a directory with the files below:
	 * + `main.c`: found below
	 * + `test.ink.json`: found at @ref src_story_json
	 *
	 * And extract `<os>-clib.zip` from the [release
	 * page](https://github.com/JBenda/inkcpp/releases/latest) to `/MY/INKCPP/EXAMPLE_INSTALL/PATH`.
	 * <br/> To run the example do the following:
	 *
	 * + change the `prefix=...` in `/MY/INKCPP/EXAMPLE_INSTALL/PATH/lib/pkgconfig/inkcpp.pc`
	 *   to `prefix=/MY/INKCPP_EXAMPLE_INSTALL_PATH/`
	 * + `export PKG_CONFIG_PATH=/MY/INKCPP/EXAMPLE_INSTALL/PATH/lib/pkgconfig`
	 * + `gcc -c main.c $(pkg-config --cflags inkcpp)`
	 * + `g++ -o main main.o $(pkg-config --libs inkcpp)`
	 * + `./main`
	 *
	 * As a sideproduct a file named `test.bin` should be created coaining the binary format used by
	 * inkCPP.
	 *
	 * @subsection src_main_c main.c
	 * @include cmake_example/main.c
	 */

	/** @class HInkSnapshot
	 * @ingroup clib
	 * @brief Handler for a @ref ink::runtime::snapshot "ink snapshot"
	 * @copydetails ink::runtime::snapshot
	 */
	struct HInkSnapshot;
	/** @memberof HInkSnapshot
	 *  @copydoc ink::runtime::snapshot::from_file()
	 */
	HInkSnapshot* ink_snapshot_from_file(const char* filename);
	/** @memberof HInkSnapshot
	 *  @copydoc  ink::runtime::snapshot::num_runners()
	 *  @param self
	 */
	int           ink_snapshot_num_runners(const HInkSnapshot* self);
	/** @memberof HInkSnapshot
	 *  @copydoc  ink::runtime::snapshot::write_to_file()
	 *  @param self
	 */
	void          ink_snapshot_write_to_file(const HInkSnapshot* self, const char* filename);

	/** @class HInkChoice
	 * @ingroup clib
	 * @brief Handler for a @ref ink::runtime::choice "ink choice"
	 * @copydetails ink::runtime::choice
	 */
	struct HInkChoice;
	/** @memberof HInkChoice
	 *  @copydoc ink::runtime::choice::text
	 *  @param self
	 */
	const char* ink_choice_text(const HInkChoice* self);
	/** @memberof HInkChoice
	 *  @copydoc ink::runtime::choice::num_tags
	 *  @param self
	 */
	int         ink_choice_num_tags(const HInkChoice* self);
	/** @memberof HInkChoice
	 *  @copydoc ink::runtime::choice::get_tag
	 *  @param self
	 */
	const char* ink_choice_get_tag(const HInkChoice* self, int index);

	/** @class HInkList
	 * @ingroup clib
	 * @brief Handler for a @ref ink::runtime::list_interface "ink list"
	 */
	struct HInkList;

	/**
	 * Iterater used to iterate flags of a ::HInkList
	 * @see ink_list_flags() ink_list_flags_from()
	 * @ingroup clib
	 * @code
	 * const HInkList* list = ...;
	 * InkListIter iter;
	 * if (ink_list_flags(list, &iter)) {
	 *  do {
	 *    iter->flag_name;
	 *    iter->list_name;
	 *    // ...
	 *  } while(ink_list_iter_next(&iter));
	 * }
	 * @endcode
	 */
	struct InkListIter {
		const void* _data;        ///< @private
		int         _i;           ///< @private
		int         _single_list; ///< @private
		const char* flag_name;    ///< Name of the current flag
		const char* list_name;    ///< name of the list the flag corresponds to
	};

	void ink_list_add(HInkList* self, const char* flag_name);
	void ink_list_remove(HInkList* self, const char* flag_name);
	int  ink_list_contains(const HInkList* self, const char* flag_name);
	/**
	 * @memberof HInkList
	 * Creates an Iterator over all flags contained in a list.
	 * @see @ref InkListIter for a usage example
	 * @retval 0 if the list contains no flags and the iterator would be invalid
	 */
	int  ink_list_flags(const HInkList* self, InkListIter* iter);
	/**
	 * @memberof HInkList
	 * Creates an Iterator over all flags contained in a list assziated with a defined list.
	 * @see @ref InkListIter for a usage example
	 * @param self
	 * @param list_name name of defined list which elements should be filterd
	 * @param[out] iter constructed iterator
	 * @retval 0 if the list contains no flags and the iterator would be invalid
	 */
	int  ink_list_flags_from(const HInkList* self, const char* list_name, InkListIter* iter);
	/**
	 * @memberof InkListIter
	 * @retval 0 if the there is no next element
	 * @retval 1 if a new flag can be found in iter.flag_name
	 */
	int  ink_list_iter_next(InkListIter* self);

	/** Repserentation of a ink variable.
	 * @ingroup clib
	 * The concret type contained is noted in @ref InkValue::type "type", please use this information
	 * to access the corresponding field of the union
	 * @attention a InkValue of type @ref InkValue::Type::ValueTypeNone "ValueTypeNone" dose not
	 * contain any value! It is use e.g. at @ref HInkGlobals::ink_globals_get() "ink_globals_get()"
	 */
	struct InkValue {
		union {
			int bool_v; ///< contains value if #type == @ref InkValue::Type::ValueTypeBool "ValueTypeBool"
			uint32_t    uint32_v; ///< contains value if #type == @ref InkValue::Type::ValueTypeUint32
			                      ///< "ValueTypeUint32"
			int32_t     int32_v;  ///< contains value if #type == @ref InkValue::Type::ValueTypeInt32
			                      ///< "ValueTypeInt32"
			const char* string_v; ///< contains value if #type == @ref InkValue::Type::ValueTypeString
			                      ///< "ValueTypeString"
			float       float_v;  ///< contains value if #type == @ref InkValue::Type::ValueTypeFloat
			                      ///< "ValueTypeFloat"
			HInkList*
			    list_v; ///< contains value if #type == @ref InkValue::Type::ValueTypeList "ValueTypeList"
		};

		/// indicates which type is contained in the value
		enum Type {
			ValueTypeNone,   ///< the Value does not contain any value
			ValueTypeBool,   ///< a boolean
			ValueTypeUint32, ///< a unsigned integer
			ValueTypeInt32,  ///< a signed integer
			ValueTypeString, ///< a string
			ValueTypeFloat,  ///< a floating point number
			ValueTypeList    ///< a ink list
		} type;            ///< indicates type contained in value
	};

	// const char* ink_value_to_string(const InkValue* self);

	/** @memberof HInkRunner
	 * Callback for a Ink external function which returns void
	 * @param argc number of arguments
	 * @param argv array containing the arguments
	 */
	typedef InkValue (*InkExternalFunction)(int argc, const InkValue argv[]);
	/** @memberof HInkRunner
	 * Callback for a Ink external function wihich returns a value
	 * @param argc number of arguments
	 * @param argv array contaning the arguments
	 * @return value to be furthe process by the ink runtime
	 */
	typedef void (*InkExternalFunctionVoid)(int argc, const InkValue argv[]);

	/** @class HInkRunner
	 * @ingroup clib
	 * A handle for an @ref ink::runtime::runner_interface "ink runner"
	 * @copydetails ink::runtime::runner_interface
	 */
	struct HInkRunner;
	/** @memberof HInkRunner
	 * Deconstructs the Runner and all frees assoziated resources
	 */
	void              ink_runner_delete(HInkRunner* self);
	/** @memberof HInkRunner
	 * Creates a snapshot, for later reloading.
	 * @attention All runners assoziated with the same globals will create the same snapshot
	 * @ref ::HInkSnapshot
	 */
	HInkSnapshot*     ink_runner_create_snapshot(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::can_continue()
	 */
	int               ink_runner_can_continue(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::getline_alloc()
	 */
	const char*       ink_runner_get_line(HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::num_tags()
	 */
	int               ink_runner_num_tags(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::get_tag()
	 * @param self
	 */
	const char*       ink_runner_tag(const HInkRunner* self, int index);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::num_knot_tags()
	 * @param self
	 */
	int               ink_runner_num_knot_tags(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::get_current_knot()
	 * @param self
	 */
	ink_hash_t        ink_runner_current_knot(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::move_to()
	 * @param self
	 * @sa ::ink_hash_string()
	 */
	bool              ink_runner_move_to(HInkRunner* self, ink_hash_t path);
	/** Hash a string, this hash is used inside inkcpp instead of the string actual value.
	 * @sa ::HInkRunner::ink_runner_move_to(), ::HInkRunner::ink_runner_current_knot()
	 * @ingroup clib
	 * @param str string to hash
	 * @return hash of string
	 */
	ink_hash_t        ink_hash_string(const char* str);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::knot_tag()
	 * @param self
	 */
	const char*       ink_runner_knot_tag(const HInkRunner* self, int index);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::num_global_tags()
	 * @param self
	 */
	int               ink_runner_num_global_tags(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::global_tag()
	 * @param self
	 */
	const char*       ink_runner_global_tag(const HInkRunner* self, int index);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::num_choices()
	 * @param self
	 */
	int               ink_runner_num_choices(const HInkRunner* self);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::get_choice()
	 * @param self
	 */
	const HInkChoice* ink_runner_get_choice(const HInkRunner* self, int index);
	/** @memberof HInkRunner
	 * @copydoc ink::runtime::runner_interface::choose()
	 * @param self
	 */
	void              ink_runner_choose(HInkRunner* self, int index);

	/** @memberof HInkRunner
	 * Binds a external function which is called form the runtime, with no return value.
	 * @see ink_runner_bind()
	 * @param self
	 * @param function_name declared in ink script
	 * @param callback
	 * @param lookaheadSafe if false stop glue lookahead if encounter this function
	 *                      this prevents double execution of external functions but can lead to
	 *                      missing glues
	 */
	void ink_runner_bind_void(
	    HInkRunner* self, const char* function_name, InkExternalFunctionVoid callback,
	    int lookaheadSafe
	);
	/** @memberof HInkRunner
	 * Binds a external function which is called from the runtime, with a return vallue.
	 * @see ink_runner_bind_void()
	 * @param self
	 * @param function_name name of external function declared inside ink script
	 * @param callback
	 * @param lookaheadSafe if false stop glue lookahead if encounter this function
	 *                      this prevents double execution of external functions but can lead to
	 *                      missing glues
	 */
	void ink_runner_bind(
	    HInkRunner* self, const char* function_name, InkExternalFunction callback, int lookaheadSafe
	);


	/** @class HInkGlobals
	 *  @ingroup clib
	 *  Handle for the @ref ink::runtime::globals_interface "global variable store" shared among ink
	 * runners.
	 *  @copydetails ink::runtime::globals_interface
	 */
	struct HInkGlobals;
	/** @memberof HInkGlobals
	 * @param new_value contains the value newly assigned
	 * @param old_value contains the previous value or a @ref InkValue::Type::ValueTypeNone
	 * "ValueTypeNone" if the variable was previously unset.
	 */
	typedef void (*InkObserver)(InkValue new_value, InkValue old_value);
	/** @memberof HInkGlobals
	 * Deconstructs the globals store and frees all assoziated memories.
	 * @attention invalidates all assoziated @ref HInkRunner
	 */
	void          ink_globals_delete(HInkGlobals* self);
	/**  @memberof HInkGlobals
	 * Creates a snapshot for later reloading.
	 * @attention All runners assoziated with the same globals will create the same snapshot.
	 * @ref ::HInkSnapshot
	 */
	HInkSnapshot* ink_globals_create_snapshot(const HInkGlobals* self);
	/** @memberof HInkGlobals
	 * assignes a observer to the variable with the corresponding name.
	 * The observer is called each time the value of the variable gets assigned.
	 * To monitor value changes compare the old with new value (see @ref InkObserver)
	 */
	void     ink_globals_observe(HInkGlobals* self, const char* variable_name, InkObserver observer);
	/**  @memberof HInkGlobals
	 * Gets the value of a global variable
	 * @param variable_name name of variable (same as in ink script)
	 * @param self
	 * @retval InkValue::Type::ValueTypeNone iff the variable does not exist
	 */
	InkValue ink_globals_get(const HInkGlobals* self, const char* variable_name);
	/**  @memberof HInkGlobals
	 * Sets the value of a globals variable.
	 * @param variable_name name of variable (same as in ink script)
	 * @param self
	 * @param value
	 * @return false if the variable was not set, because the variable with this name does no exists
	 * or the value did not match.
	 */
	int      ink_globals_set(HInkGlobals* self, const char* variable_name, InkValue value);

	/** @class HInkStory
	 *  @ingroup clib
	 *  Handle for a loaded @ref ink::runtime::story "ink story"
	 *  @copydetails ink::runtime::story
	 *  @see HInkGlobals
	 *  @see HInkRunner
	 */
	struct HInkStory;
	/** @memberof HInkStory
	 *  @copydoc ink::runtime::story::from_file
	 */
	HInkStory*   ink_story_from_file(const char* filename);
	/** @memberof HInkStory
	 * deletes a story and all assoziated resources
	 * @param self
	 * @attention this will invalidate all ::HInkRunner and ::HInkGlobals handles assoziated with this
	 * story
	 */
	void         ink_story_delete(HInkStory* self);
	/** @memberof HInkStory
	 *  @copydoc ink::runtime::story::new_globals
	 *  @param self
	 */
	HInkGlobals* ink_story_new_globals(HInkStory* self);
	/** @memberof HInkStory
	 *  @copydoc ink::runtime::story::new_runner
	 *  @param self
	 */
	HInkRunner*  ink_story_new_runner(HInkStory* self, HInkGlobals* store);
	/** @memberof HInkStory
	 *  @copydoc ink::runtime::story::new_globals_from_snapshot
	 *  @param self
	 */
	HInkGlobals* ink_story_new_globals_from_snapshot(HInkStory* self, const HInkSnapshot* obj);
	/** @memberof HInkStory
	 *  @copydoc ink::runtime::story::new_runner_from_snapshot
	 *  @param self
	 */
	HInkRunner*  ink_story_new_runner_from_snapshot(
	     HInkStory* self, const HInkSnapshot* obj, HInkGlobals* store, int runner_id
	 );

	/**
	 * @ingroup clib
	 * Compiles a .ink.json file to an inkCPP .bin file.
	 * @param input_filename path to file contaning input data (.ink.json)
	 * @param output_filename path to file output data will be written (.bin)
	 * @param error if not NULL will contain a error message if an error occures (else will be set to
	 * NULL)
	 */
	void
	    ink_compile_json(const char* input_filename, const char* output_filename, const char** error);


#ifdef __cplusplus
}
#endif

#endif
