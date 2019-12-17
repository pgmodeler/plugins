//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lcp.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-08-29
 */
#ifndef PAAL_LCP_HPP
#define PAAL_LCP_HPP

#include <vector>

namespace paal {
/**
 *
 * @brief
 * fill array lcp
 * lcp[0] is undefined
 * and
 * lcp[i] stores the largest common prefix of the lexicographically i-1'th
* smallest suffix
 * and its predecessor in the suffix array
 * @tparam Letter
 * @param suffix_array
 * @param rank
 * @param lcp place for Lcp
 * @param sumWords
 */
template <typename Letter>
void lcp(std::vector<int> const &suffix_array, std::vector<int> const &rank,
         std::vector<int> &lcp, std::vector<Letter> const &sumWords) {
    int comonPrefixLength = 0;
    for (auto suffixRank :
         rank) { // suffixRank number suffix in lexicographically order
        if (suffixRank != 0) {
            while (sumWords[suffix_array[suffixRank] + comonPrefixLength] ==
                   sumWords[suffix_array[suffixRank - 1] + comonPrefixLength]) {
                ++comonPrefixLength;
            }
        }
        lcp[suffixRank] = comonPrefixLength;
        if (comonPrefixLength > 0) {
            --comonPrefixLength;
        }
    }
}
} //!paal

#endif // PAAL_LCP_HPP
