//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file suffix_array.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-08-29
 */
#ifndef PAAL_SUFFIX_ARRAY_HPP
#define PAAL_SUFFIX_ARRAY_HPP

#include "paal/utils/irange.hpp"

#include <boost/range/algorithm/fill.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/numeric.hpp>

/*
 * algorithm from:
 *
 * http://www.cs.cmu.edu/~guyb/realworld/papersS04/KaSa03.pdf
 *
 */

namespace paal {

namespace detail {
/**
 * @param a1
 * @param a2
 * @param b1
 * @param b2
 * @brief return true if pair (a1,a2) is smaller than pair (b1,b2) in
 * lexicographic order
 * and false otherwise
 * @tparam Letter
 */
// class suffix_array{
template <typename Letter>
inline bool leq(Letter a1, int a2, Letter b1,
                int b2) { // lexic. order for pairs
    return (a1 < b1 || (a1 == b1 && a2 <= b2));
}
/**
 * @param a1
 * @param a2
 * @param a3
 * @param b1
 * @param b2
 * @param b3
 * @brief return true if triple (a1,a2,a3) is smaller than triple (b1,b2,b3) in
 * lexicographic order
 * and false otherwise
 * @tparam Letter
 */
template <typename Letter>
inline bool leq(Letter a1, Letter a2, int a3, Letter b1, Letter b2, int b3) {
    return (a1 < b1 || (a1 == b1 && leq(a2, a3, b2, b3)));
}

struct radix_pass {

    template <typename Letter>
    radix_pass(int max_letter, const std::vector<Letter> & text) {
        if (max_letter == 0) {
            max_letter = *boost::max_element(text);
        }
        c.resize(max_letter + 2);
    }

    // stably sort sortFrom[0..n-1] to sortTo[0..n-1] with keys in 0..K from r
    template <typename Iterator>
    void operator()(std::vector<int> const &sortFrom,
            std::vector<int> &sortTo, Iterator r, int n) { // count occurrences
        boost::fill(c, 0);
        for (auto i : irange(n)) {
            ++c[r[sortFrom[i]] + 1]; // count occurrences
        }

        boost::partial_sum(c, c.begin());

        for (auto i : irange(n)) {
            sortTo[c[r[sortFrom[i]]]++] = sortFrom[i]; // sort
        }
    }
private:
    std::vector<int> c;
};

/**
 *
 * @brief
 * require text[n]=text[n+1]=text[n+2]=0, n>=2
 * fill suffix_array
 * suffix_array[i] contains the starting position of the i-1'th smallest suffix
* in Word
 * @tparam Letter
 * @param text - text
 * @param SA place for suffix_array
 * @param max_letter optional parameter max_letter in alphabet
 */
// find the suffix array SA of text[0..n-1] in {1..max_letter}^n
template <typename Letter>
void suffix_array(std::vector<Letter> &text, std::vector<int> &SA,
                   Letter max_letter) {
    int n = text.size() - 3;
    assert(n >= 0);
    int n0 = (n + 2) / 3, n1 = (n + 1) / 3, n2 = n / 3, n02 = n0 + n2;
    text.resize(text.size() + 3);
    std::vector<int> text12;
    std::vector<int> SA12;
    std::vector<int> text0;
    std::vector<int> SA0;
    radix_pass radix{max_letter, text};
    // generate positions of mod 1 and mod  2 suffixes
    // the "+(n0-n1)" adds a dummy mod 1 suffix if n%3 == 1
    for (auto i : irange(n + (n0 - n1))) {
        if (i % 3 != 0) {
            text12.push_back(i);
        }
    }
    SA0.resize(n0);
    SA12.resize(n02 + 3);
    text12.resize(n02 + 3);
    // lsb radix sort the mod 1 and mod 2 triples
    radix(text12, SA12, text.begin() + 2, n02);
    radix(SA12, text12, text.begin() + 1, n02);
    radix(text12, SA12, text.begin(), n02);

    // find lexicographic names of triples
    int name = 0;
    Letter c0 = Letter{}, c1 = Letter{}, c2 = Letter{};
    for (auto i : irange(n02)) {
        if (text[SA12[i]] != c0 || text[SA12[i] + 1] != c1 ||
            text[SA12[i] + 2] != c2 || name == 0) {
            name++;
            c0 = text[SA12[i]];
            c1 = text[SA12[i] + 1];
            c2 = text[SA12[i] + 2];
        }
        if (SA12[i] % 3 == 1) {
            text12[SA12[i] / 3] = name; // left half
        } else {
            text12[SA12[i] / 3 + n0] = name; // right half
        }
    }

    // recurse if names are not yet unique
    if (name < n02) {
        suffix_array<int>(text12, SA12, name); // parametrized by int intentionally
        // store unique names in s12 using the suffix array
        for (auto i : irange(n02)) {
            text12[SA12[i]] = i + 1;
        }
    } else { // generate the suffix array of s12 directly
        for (auto i : irange(n02)) {
            SA12[text12[i] - 1] = i;
        }
    }

    // stably sort the mod 0 suffixes from SA12 by their first character
    for (auto i : irange(n02)) {
        if (SA12[i] < n0) {
            text0.push_back(3 * SA12[i]);
        }
    }
    radix(text0, SA0, text.begin(), n0);
    auto GetI = [&](int t)->int {
        return SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2;
    };

    // merge sorted SA0 suffixes and sorted SA12 suffixes
    auto p = SA0.begin();
    int t = n0 - n1;
    for (auto k = SA.begin(); k < SA.begin() + n; k++) {
        int i = GetI(t); // pos of current offset 12 suffix
        int j = (*p);    // pos of current offset 0  suffix
        if (SA12[t] < n0
                ? leq(text[i], text12[SA12[t] + n0], text[j], text12[j / 3])
                : leq(text[i], text[i + 1], text12[SA12[t] - n0 + 1], text[j],
                      text[j + 1], text12[j / 3 + n0])) { // suffix from SA12 is
                                                          // smaller
            (*k) = i;
            t++;
            if (t == n02) { // done --- only SA0 suffixes left
                k++;
                if (p < SA0.end()) {
                    k = std::copy(p, SA0.end(), k);
                    p = SA0.end();
                }
            }
        } else {
            (*k) = j;
            p++;
            if (p == SA0.end()) { // done --- only SA12 suffixes left
                for (k++; t < n02; t++, k++) {
                    (*k) = GetI(t);
                }
            }
        }
    }
}

}//!detail


/**
 *
 * @brief
 * require text.size()>=2
 * fill suffix_array
 * suffix_array[i] contains the starting position of the i-1'th smallest suffix
* in Word
 * @tparam Letter
 * @param text - text
 * @param SA place for suffix_array
 * @param max_letter optional parameter max_letter in alphabet
 */
// find the suffix array SA of text[0..n-1] in {1..max_letter}^n
template <typename Letter>
void suffix_array(std::vector<Letter> &text, std::vector<int> &SA,
                  Letter max_letter = 0) {
    text.resize(text.size() + 3);
    detail::suffix_array(text, SA, max_letter);
    text.resize(text.size() - 3);
}

} //!paal
#endif // PAAL_SUFFIX_ARRAY_HPP
