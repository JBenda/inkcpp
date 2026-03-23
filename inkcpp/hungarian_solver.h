#pragma once

/** Hungarian Algorithm to solve an assignment problem in O(N3).
 * https://en.wikipedia.org/wiki/Hungarian_algorithm
 * @param[in] cost matrix m x n
 * @param[out] matches optimal mapping m -> n
 * @param n number of jobs/assignments
 */
float hungarian_solver(const float* cost, int* matches, size_t n);
