/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "types.h"

namespace ink::runtime
{
/**
 * Container for an InkCPP runtime snapshot.
 * Each snapshot contains a @ref ink::runtime::globals_interface "globals store"
 * and all associated @ref ink::runtime::runner_interface "runners/threads"
 * For convinience there exist @ref ink::runtime::globals_interface::create_snapshot() and
 * runner_interface::create_snapshot() . If the runner is associated to the globals the snapshot
 * will be identical. If multiple runners are associated to the same globals all will be contained,
 * and cann be reconsrtucted with the id parameter of @ref
 * ink::runtime::story::new_runner_from_snapshot()
 * A snapshot can be applied to an identical story file or an simulare if the snapshot is @ref
 * ink::runtime::snapshot::can_be_migrated() "@c can_be_migrated()".
 * A not migrated snapshot contiouse at exactly the place you are currently at.
 *
 * **A migrated one will "snap bag" to the last knot.**
 *
 * + Global variables which (name) still exist will be transfared.
 *   + New ones will be initelized with its default value
 *   + Old ones will be droped
 * + Temp variables which (name) still exist will be tranfared
 *   + new ones will be initelized with its default vaule (possible missing transformations)
 *   + old ones will be kept
 *   + **attention** declarations in Tunnels will be missed
 * + Stack/Threads/Tunnels must not be used in the moment of the snapshot for it to be migratable.
 *   + best practice is to create hub knots which names do not change in the progress of the update
 *     and which do not have local variables. Then only store after you stepped inside this knot.
 * + Lists definitions are matched after best knowladge
 *   + for each pair of old list value and new list value the best good matching is used
 *     + the similarty is calculated based on the jaro-winkler similiarty of the value names, and
 *       the normalized difference of the values
 *   +  for each pair of old and new list (definition) the best good match is taken
 *     + the similiraty is calculated based on the jaccard similiraty of the contained flags, and
 *       the jaro-winkler similarty of the lists names
 * + visit counts (e.g. used for once only choices)
 *   + existing ones (exact name match) are kept
 *     + !! a choice with no explicit label is tracked via its position in the list, reordering
 *       choices can therfore break your visit counts
 *   + new ones are zero
 *   + old ones are discarded
 *
 * @todo Currently the id is equal to the creation order, a way to name the single runner/threads is
 * WIP
 */
class snapshot
{
public:
	virtual ~snapshot(){};

	/** Construct snapshot from blob.
	 * Memory must be kept valid until the snapshot is deconstructed.
	 * @param data pointer to blob
	 * @param length number of bytes in blob
	 * @param freeOnDestroy if the memory should be freed (delete[]) when the snapshot is
	 * deconstructed
	 * @return newly created snapshot
	 */
	static snapshot* from_binary(const unsigned char* data, size_t length, bool freeOnDestroy = true);

	/** access blob inside snapshot */
	virtual const unsigned char* get_data() const        = 0;
	/** size of blob inside snapshot */
	virtual size_t               get_data_len() const    = 0;
	/** number of runners which are stored inside this snapshot */
	virtual size_t               num_runners() const     = 0;
	/** if this snapshot can be migrated, if the story file changes (slightly). */
	virtual bool                 can_be_migrated() const = 0;

#ifdef INK_ENABLE_STL
	/** deserialize snapshot from file.
	 * @param filename of input file
	 * @throws ink_exception if it fails to open the file
	 */
	static snapshot* from_file(const char* filename);
	/** serialize snapshot to file
	 * @param filename output file filename, if already exist it will be overwritten
	 * @throws ink_exception if it fails to open the file
	 */
	void             write_to_file(const char* filename) const;
#endif
};
} // namespace ink::runtime
