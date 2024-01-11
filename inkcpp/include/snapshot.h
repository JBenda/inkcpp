#pragma once

#include "types.h"

namespace ink::runtime
{
/**
 * Container for an InkCPP runtime snapshot.
 * Each snapshot contains a @ref globals_interface "globals store"
 * and all assoziated @ref runner_interface "runners/threads"
 * For convinience there exist @ref globals_interface::create_snapshot() and
 * runner_interface::create_snapshot() . If the runner is assoziated to the globals the snapshot
 * will be identical. If multiple runners are assoziated to the same globals all will be contained,
 * and cann be reconsrtucted with the id parameter of @ref story::new_runner_from_snapshot()
 *
 * @todo Currently the id is equal to the creation order, a way to name the single runner/threads is
 * WIP
 */
class snapshot
{
public:
	virtual ~snapshot(){};

	static snapshot* from_binary(const unsigned char* data, size_t length, bool freeOnDestroy = true);

	virtual const unsigned char* get_data() const     = 0;
	virtual size_t               get_data_len() const = 0;
	virtual size_t               num_runners() const  = 0;

#ifdef INK_ENABLE_STL
	static snapshot* from_file(const char* filename);
	void             write_to_file(const char* filename) const;
#endif
};
} // namespace ink::runtime
