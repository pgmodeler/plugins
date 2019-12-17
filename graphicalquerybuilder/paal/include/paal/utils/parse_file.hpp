//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file parse_file.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-12-12
 */
#ifndef PAAL_PARSE_FILE_HPP
#define PAAL_PARSE_FILE_HPP

#include <cassert>
#include <fstream>
#include <istream>
#include <limits>

namespace paal {

/**
 * @brief parses stream ignoring empty lines or beginning with '#'
 *
 * @tparam Functor
 * @param input_stream
 * @param f functor called on each line with params: first token of line and
 * stream after reading that token
 */
template <typename Functor> void parse(std::istream &input_stream, Functor f) {
    assert(input_stream.good());
    while (input_stream.good()) {
        std::string first_token;
        input_stream >> first_token;
        if (first_token == "") {
            return;
        }
        if (first_token[0] == '#') {
            input_stream.ignore(std::numeric_limits<std::streamsize>::max(),
                                '\n');
            continue;
        }
        f(first_token, input_stream);
    }
}

/**
 * @brief additional version of parse function
 * @tparam Functor
 * @param filename
 * @param f
 */
template <typename Functor> void parse(const std::string &filename, Functor f) {
    std::ifstream input_stream(filename);
    parse(input_stream, f);
}

} //! paal
#endif // PAAL_PARSE_FILE_HPP
