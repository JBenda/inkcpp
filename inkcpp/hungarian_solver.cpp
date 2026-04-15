#include "hungarian_solver.h"

#include "system.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <stdint.h>

class HungarienCtx
{
	const int    n;
	const float* cost;

	struct {
		float* row;
		float* col;
	} pot;

	float* slack;
	int*   col_2_row;
	int*   path;
	bool*  visit_col;

public:
	HungarienCtx(const float* cost, int n)
	    : n{n}
	    , cost{cost}
	{
		pot.row = new float[n + 1];
		memset(pot.row, 0, sizeof(float) * (n + 1));
		pot.col = new float[n + 1];
		memset(pot.col, 0, sizeof(float) * (n + 1));
		slack     = new float[n + 1];
		col_2_row = new int[n + 1];
		memset(col_2_row, 0, sizeof(int) * (n + 1));
		path      = new int[n + 1];
		visit_col = new bool[n + 1];
	}

	~HungarienCtx()
	{
		delete[] pot.row;
		delete[] pot.col;
		delete[] slack;
		delete[] col_2_row;
		delete[] path;
		delete[] visit_col;
	}

	int operator[](int col) { return col_2_row[col]; }

	void init_search(int row)
	{
		col_2_row[0] = row;
		for (size_t i = 0; i <= static_cast<size_t>(n); ++i) {
			visit_col[i] = false;
			slack[i]     = std::numeric_limits<float>::max();
			path[i]      = 0;
		}
	}

	int find_augmenting_path()
	{
		int current_col = 0;
		do {
			visit_col[current_col] = true;
			int   current_row      = col_2_row[current_col];
			float delta            = std::numeric_limits<float>::max();
			int   next_col         = 0;
			for (int col = 1; col <= n; ++col) {
				if (! visit_col[col]) {
					float reduced
					    = cost[(current_row - 1) * n + (col - 1)] - pot.row[current_row] - pot.col[col];
					if (reduced < slack[col]) {
						slack[col] = reduced;
						path[col]  = current_col;
					}
					if (slack[col] < delta) {
						delta    = slack[col];
						next_col = col;
					}
				}
			}

			for (size_t col = 0; col <= static_cast<size_t>(n); ++col) {
				if (visit_col[col]) {
					pot.row[col_2_row[col]] += delta;
					pot.col[col] -= delta;
				} else {
					slack[col] -= delta;
				}
			}
			current_col = next_col;
		} while (col_2_row[current_col]);
		return current_col;
	}

	// end_col = 0 -> no augmenting
	void augment_matching(int end_col)
	{
		int col = end_col;
		do {
			int prev       = path[col];
			col_2_row[col] = col_2_row[prev];
			col            = prev;
		} while (col != 0);
	}
};

namespace ink::algorithms
{
float hungarian_solver(const float* cost, int* matches, size_t n, float threshold)
{
	HungarienCtx ctx(cost, n);
	for (size_t row = 1; row <= n; ++row) {
		ctx.init_search(static_cast<int>(row));
		int end_col = ctx.find_augmenting_path();
		ctx.augment_matching(end_col);
	}

	float total_cost = 0;
	for (size_t col = 1; col <= n; ++col) {
		int row      = ctx[col] - 1;
		matches[row] = col - 1;
		total_cost += cost[row * n + matches[row]];
		if (threshold != 0 && cost[row * n + matches[row]] >= threshold) {
			matches[row] = -1;
		}
	}
	return total_cost;
}

float jaro_simularity(const char* lh, const char* rh)
{
	const size_t lh_len          = static_cast<size_t>(strlen(lh));
	uint8_t      lh_matched[256] = {};
	if (lh_len > sizeof(lh_matched) * 8) {
		return 0;
	}
	const size_t rh_len          = static_cast<size_t>(strlen(rh));
	uint8_t      rh_matched[256] = {};
	if (rh_len > sizeof(rh_matched) * 8) {
		return 0;
	}

	if ((lh_len == 1 && rh_len == 1) || lh_len == 0 || rh_len == 0) {
		return 0;
	}
	size_t max_offset = (std::max(lh_len, rh_len) / 2) - 1;
	float  m          = 0;

	for (int lh_idx = 0; static_cast<size_t>(lh_idx) < lh_len; ++lh_idx) {
		for (int rh_idx = std::max(lh_idx - static_cast<int>(max_offset), 0);
		     static_cast<size_t>(rh_idx) <= std::min(rh_len, lh_idx + max_offset); ++rh_idx) {
			if (! (rh_matched[rh_idx / 8] & (1 << (rh_idx & 7)))
			    && tolower(rh[rh_idx]) == tolower(lh[lh_idx])) {
				lh_matched[lh_idx / 8] |= 1 << (lh_idx & 7);
				rh_matched[rh_idx / 8] |= 1 << (rh_idx & 7);
				m += 1.;
				break;
			}
		}
	}

	if (m == 0) {
		return 0;
	}
	int   rh_idx = 0;
	float t      = 0;
	for (int lh_idx = 0; static_cast<size_t>(lh_idx) < lh_len; ++lh_idx) {
		if (lh_matched[lh_idx / 8] & (1 << (lh_idx & 7))) {
			int next_idx = rh_idx;
			while (static_cast<size_t>(next_idx) < rh_len) {
				if (rh_matched[next_idx / 8] & 1 << (next_idx & 7)) {
					rh_idx = next_idx + 1;
					break;
				}
				next_idx += 1;
			}
			if (tolower(lh[lh_idx]) != tolower(rh[next_idx])) {
				t += 1.;
			}
		}
	}
	t /= 2.;
	return ((m / lh_len) + (m / rh_len) + ((m - t) / m)) / 3.f;
}

static constexpr float P = 0.1f;

float jaro_winkler_simularity(const char* lh, const char* rh)
{
	float       j = jaro_simularity(lh, rh);
	int         l = 0;
	const char *l_iter, *r_iter;
	// calculate length of common prefix
	for (l_iter = lh, r_iter = rh; *l_iter && *r_iter && *l_iter == *r_iter; ++lh, ++rh) {
		l += 1;
		if (l == 4) {
			break;
		}
	}
	return j + l * P * (1 - j);
}
} // namespace ink::algorithms
