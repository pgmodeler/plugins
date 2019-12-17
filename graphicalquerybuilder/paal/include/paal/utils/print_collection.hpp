//=======================================================================
// Copyright (c) 2014 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file print_collection.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-06-12
 */

#ifndef PAAL_PRINT_COLLECTION_HPP
#define PAAL_PRINT_COLLECTION_HPP

#include <boost/range/empty.hpp>
#include <boost/range.hpp>

namespace paal {

/**
 * @brief prints collection with delimiters without trailing delimiter
 *
 * @tparam Range
 * @tparam Stream
 * @param o
 * @param r
 * @param del
 */
template <typename Range, typename Stream>
void print_collection(Stream &o, Range &&r, const std::string &del) {
    auto b = std::begin(r);
    auto e = std::end(r);
    if (b == e) {
        return;
    }
    o << *b;
    for (auto &&x : boost::make_iterator_range(++b, e)) {
        o << del << x;
    }
}

/**
 * @brief prints matrix with delimiters
 *
 * @tparam Matrix
 * @tparam Stream
 * @param o
 * @param m
 * @param del
 */
template <typename Matrix, typename Stream>
void print_matrix(Stream &o, Matrix &&m, const std::string &del) {
    auto b = m.begin1();
    auto e = m.end1();
    if (b == e) {
        return;
    }
    print_collection(o, b, del);
    for (++b ;b != e; ++b) {
        o << std::endl;
        print_collection(o, b, del);
    }
}

} //!paal

#endif // PAAL_PRINT_COLLECTION_HPP
