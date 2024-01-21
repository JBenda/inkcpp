#pragma once

#include "types.h"

namespace ink::runtime
{
/**
 * Container for an InkCPP runtime snapshot.
 * Each snapshot contains a @ref ink::runtime::globals_interface "globals store"
 * and all assoziated @ref ink::runtime::runner_interface "runners/threads"
 * For convinience there exist @ref ink::runtime::globals_interface::create_snapshot() and
 * runner_interface::create_snapshot() . If the runner is assoziated to the globals the snapshot
 * will be identical. If multiple runners are assoziated to the same globals all will be contained,
 * and cann be reconsrtucted with the id parameter of @ref
 * ink::runtime::story::new_runner_from_snapshot()
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

	/** acces blob inside snapshot */
	virtual const unsigned char* get_data() const     = 0;
	/** size of blob inside snapshot */
	virtual size_t               get_data_len() const = 0;
	/** number of runners which are stored inside this snapshot */
	virtual size_t               num_runners() const  = 0;

#ifdef INK_ENABLE_STL
	/** deserialize snapshot from file.
	 * @param filename of input file
	 * @throws ink_exception if it fails to open the file
	 */
	static snapshot* from_file(const char* filename);
	/** serialize snapshot to file
	 * @param filename output file filename, if already exist it will be overwritten
	 * @throws ink_exception if it failt to open the file
	 */
	void             write_to_file(const char* filename) const;
#endif
};
} // namespace ink::runtime
