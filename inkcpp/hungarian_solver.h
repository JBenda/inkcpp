#pragma once

/** Jaro Similarity of two null terminated byte strings.
 * supports ASCII encoding, UTF-8 might be broken.
 * https://en.wikipedia.org/wiki/Jaro%E2%80%93Winkler_distance#Jaro_similarity
 * @param lh,rh null terminated byte strings to compare
 * @return similarity between lh and rh
 * @retval 1 if equal
 */
float jaro_simularity(const char* lh, const char* rh);

/** Hungarian Algorithm to solve an assignment problem in O(N3).
 * https://en.wikipedia.org/wiki/Hungarian_algorithm
 * @param[in] cost matrix m x n
 * @param[out] matches optimal mapping m -> n
 * @param n number of jobs/assignments
 * @return total cost of assigment
 */
float hungarian_solver(const float* cost, int* matches, size_t n);
