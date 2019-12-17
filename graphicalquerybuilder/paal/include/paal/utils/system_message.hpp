//=======================================================================
// Copyright (c) 2015
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file system_message.hpp
 * @brief
 * @author Tomasz Strozak
 * @version 1.0
 * @date 2015-06-26
 */
#ifndef PAAL_SYSTEM_MESSAGE_HPP
#define PAAL_SYSTEM_MESSAGE_HPP

#include <string>
#include <cstdlib>
#include <iostream>

namespace paal {

namespace utils {

/// prints message (specialization for empty message)
inline auto print_message(std::ostream &output_stream) {
    output_stream << std::endl;
}

/// prints message
template <typename Arg, typename ...Args>
auto print_message(std::ostream &output_stream, Arg &&arg, Args... args) {
    output_stream << arg;
    print_message(output_stream, std::forward<Args>(args)...);
}

/// prints info message
template <typename Arg, typename ...Args>
auto info(Arg &&arg, Args... args) {
    print_message(std::cout, std::forward<Arg>(arg), std::forward<Args>(args)...);
}

/// prints warning message
template <typename Arg, typename ...Args>
auto warning(Arg &&arg, Args... args) {
    static const std::string message_prefix = "Warning: ";
    print_message(std::cerr, message_prefix, std::forward<Arg>(arg), std::forward<Args>(args)...);
}

/// prints failure message
template <typename Arg, typename ...Args>
auto failure(Arg &&arg, Args... args) {
    static const std::string message_prefix = "Failure: ";
    print_message(std::cerr, message_prefix, std::forward<Arg>(arg), std::forward<Args>(args)...);
    std::exit(EXIT_FAILURE);
}

/// Functor prints failure message
struct failure_message {
    /// operator()
    template <typename Arg, typename ...Args>
    void operator()(Arg &&arg, Args... args) {
        failure(std::forward<Arg>(arg), std::forward<Args>(args)...);
    }
};

} // utils

} // paal

#endif // PAAL_SYSTEM_MESSAGE_HPP

