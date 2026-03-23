#include "hungarian_solver.h"
#include <limits>

class HungarienCtx
{
	const size_t n;
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
	HungarienCtx(const float* cost, size_t n)
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

	void init_search(size_t row)
	{
		col_2_row[0] = row;
		for (size_t i = 0; i <= n; ++i) {
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
			for (size_t col = 1; col <= n; ++col) {
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

			for (size_t col = 0; col <= n; ++col) {
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

float hungarian_solver(const float* cost, int* matches, size_t n)
{
	HungarienCtx ctx(cost, n);
	for (size_t row = 1; row <= n; ++row) {
		ctx.init_search(row);
		int end_col = ctx.find_augmenting_path();
		ctx.augment_matching(end_col);
	}

	float total_cost = 0;
	for (size_t col = 1; col <= n; ++col) {
		int row      = ctx[col] - 1;
		matches[row] = col - 1;
		total_cost += cost[row * n + matches[row]];
	}
	return total_cost;
}
