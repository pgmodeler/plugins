//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file shortest_superstring_example.cpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-08-29
 */

    //! [Shortest Superstring Example]
#include "paal/greedy/shortest_superstring/shortest_superstring.hpp"

#include <iostream>
#include <string>

/**
 * @brief show how to use shortest_superstring
 */
int main() {
    std::vector<std::string> words({ "ba", "ab", "aa", "bb" });

    std::cout << paal::greedy::shortestSuperstring(words) << std::endl;
}
    //! [Shortest Superstring Example]
