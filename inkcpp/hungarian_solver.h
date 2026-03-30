#pragma once

/** Jaro Similarity of two null terminated byte strings.
 * supports ASCII encoding, UTF-8 might be broken.
 * ignores case.
 * https://en.wikipedia.org/wiki/Jaro%E2%80%93Winkler_distance#Jaro_similarity
 * @param lh,rh null terminated byte strings to compare
 * @return similarity between lh and rh
 * @retval 1 if equal
 */
float jaro_simularity(const char* lh, const char* rh);

/** Jaro Winkler Similarity of two null terminated byte strings.
 * supports ASCII encoding, UTF-8 might be broken.
 * ignores case.
 * https://en.wikipedia.org/wiki/Jaro%E2%80%93Winkler_distance#Jaro%E2%80%93Winkler_similarity
 * @param lh,rh null terminated byte strings to compare
 * @sa jaro_simularity
 * @return similarity between lh and rh
 * @retval 1 if equal
 */
float jaro_winkler_simularity(const char* lh, const char* rh);

/** Hungarian Algorithm to solve an assignment problem in O(N3).
 * https://en.wikipedia.org/wiki/Hungarian_algorithm
 * @param[in] cost matrix m x n
 * @param[out] matches optimal mapping m -> n
 * @param threshold matches with a value higher than threshold will be set to `-1`, use 0 to ignore.
 * @param n number of jobs/assignments
 * @return total cost of assigment
 */
float hungarian_solver(const float* cost, int* matches, size_t n, float threshold = 0);
