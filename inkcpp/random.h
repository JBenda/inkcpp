#pragma once

#include "system.h"

namespace ink::runtime::internal {
	/**
	 * @brief pseudo random number generator based on Linear Congruential Generator (using glibc default values).
	 */
	class prng {
		static constexpr uint64_t C = 12345;
		static constexpr uint64_t A = 1103515245;
		static constexpr uint64_t M = 1ul<<31;
	public:
		void srand(uint32_t seed) {
			_x = seed;
		}
		uint32_t rand() {
			_x = static_cast<uint32_t>((A*_x+ C) % M);
			return _x;
		}
		uint32_t rand(uint32_t max) {
			uint64_t prod = rand();
			prod *= max;
			return static_cast<int32_t>(prod / M);
		}
		uint32_t get_state() const { return _x; }
		prng(uint32_t seed) : _x{seed}{}
		prng() : prng(1337) {}
	private:
		uint32_t _x;
	};
}
