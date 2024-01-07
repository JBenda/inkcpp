#ifndef _INKCPP_H
#define _INKCPP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	/** @defgroup clib Clib Interface
	 * Clib Interface: <insert example?>
	 */

	/** @class HInkSnapshot
	 * @ingroup clib
	 * @brief Handler for a ink snapshot
	 * @copydoc ink::runtime::snapshot
	 */
	struct HInkSnapshot;
	/** @memberof HInkSnapshot
	 *  @copydoc ink::runtime::snapshot::from_file
	 */
	HInkSnapshot* ink_snapshot_from_file(const char* filename);
	int           ink_snapshot_num_runners(const HInkSnapshot* self);
	void          ink_snapshot_write_to_file(const HInkSnapshot* self, const char* filename);

	struct HInkChoice;
	const char* ink_choice_text(const HInkChoice* self);
	int         ink_choice_num_tags(const HInkChoice* self);
	const char* ink_choice_get_tag(const HInkChoice* self, int tag_id);

	struct HInkList;
	struct HInkListIter;

	struct InkFlag {
		const char* flag_name;
		const char* list_name;
	};

	void          ink_list_add(HInkList* self, const char* flag_name);
	void          ink_list_remove(HInkList* self, const char* flag_name);
	bool          ink_list_contains(const HInkList* self, const char* flag_name);
	HInkListIter* ink_list_flags(const HInkList* self);
	HInkListIter* ink_list_flags_from(const HInkList* self, const char* list_name);
	InkFlag       ink_list_iter_next(HInkListIter* self);

	/** Repserentation of a ink variable.
	 * @ingroup clib
	 * The concret type contained is noted in @ref InkValue::type "type", please use this information
	 * to access the corresponding field of the union
	 */
	struct InkValue {
		union {
			int         bool_v; ///< contains value if #type == @ref InkValue::Type::Bool "Bool"
			uint32_t    uint32_v; ///< contains value if #type == @ref InkValue::Type::Uint32 "Uint32"
			int32_t     int32_v; ///< contains value if #type == @ref InkValue::Type::Int32 "Int32"
			const char* string_v; ///< contains value if #type == @ref InkValue::Type::String "String"
			HInkList*   list_v; ///< contains value if #type == @ref InkValue::Type::List "List"
		};

		/// indicates which type is contained in the value
		enum Type {
			Bool, ///< a boolean
			Uint32, ///< a unsigned integer
			Int32, ///< a signed integer
			String, ///< a string
			Float, ///< a floating point number
			List ///< a ink list
		} type; ///< indicates type contained in value
	};

	const char* ink_value_to_string(const InkValue* self);

	/** @memberof HInkRunner
	 * Callback for a Ink external function which returns void
	 * @param argc number of arguments
	 * @param argv array containing the arguments
	 */
	typedef void (*InkExternalFunctionVoid)(int argc, const InkValue* argv);
	/** @memberof HInkRunner
	 * Callback for a Ink external function wihich returns a value
	 * @param argc number of arguments
	 * @param argv array contaning the arguments
	 * @return value to be furthe process by the ink runtime
	 */
	typedef InkValue (*InkExternalFunction)(int argc, const InkValue* argv);

	/** @class HInkRunner
	 * @ingroup clib
	 */
	struct HInkRunner;
	HInkSnapshot*     ink_runner_create_snapshot(const HInkRunner* self);
	int               ink_runner_can_continue(const HInkRunner* self);
	const char*       ink_runner_get_line(HInkRunner* self);
	const char*       ink_runner_get_all(HInkRunner* self);
	int               ink_runner_num_tags(const HInkRunner* self);
	const char*       ink_runner_tag(const HInkRunner* self, int tag_id);
	int               ink_runner_num_choices(const HInkRunner* self);
	const HInkChoice* ink_runner_get_choice(const HInkRunner* self, int choice_id);
	void              ink_runner_choose(HInkRunner* self, int choice_id);
	void              ink_runner_bind_void(HInkRunner* self, InkExternalFunctionVoid callback);
	void              ink_runner_bind(HInkRunner* self, InkExternalFunction callback);

	typedef void (*InkObserver)(InkValue new_value, const InkValue* old_value);

	/** @class HInkGlobals
	 *  @ingroup clib
	 *  Handle for the @ref ink::runtime::globals_interface "global variable store" shared among ink
	 * runners.
	 *  @copydetails ink::runtime::globals_interface
	 */
	struct HInkGlobals;
	/**  @memberof HInkGlobals */
	HInkSnapshot* ink_globals_create_snapshot(const HInkGlobals* self);
	/** @memberof HInkGlobals */
	void          ink_globals_observe(HInkGlobals* self, InkObserver* observer);
	/**  @memberof HInkGlobals */
	InkValue      ink_globals_get(const HInkGlobals* self, const char* variable_name);
	/**  @memberof HInkGlobals */
	bool          ink_globals_set(HInkGlobals* self, const char* variable_name, InkValue value);

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
	 *  @copydoc ink::runtime::story::new_globals
	 */
	HInkGlobals* ink_story_new_globals(HInkStory* self);
	HInkRunner*  ink_story_new_runner(HInkStory* self, HInkGlobals* globals);
	HInkGlobals* ink_story_new_globals_from_snapshot(HInkStory* self, const HInkSnapshot* snapshot);
	HInkRunner*  ink_story_runner_from_snapshot(
	     HInkStory* self, const HInkSnapshot* snapshot, HInkGlobals* globals
	 );

	void
	    ink_compile_json(const char* input_filename, const char* output_filename, const char** error);


#ifdef __cplusplus
}
#endif

#endif
