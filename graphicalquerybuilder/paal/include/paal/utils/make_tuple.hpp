//=======================================================================
// Copyright (c) 2014 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file make_tuple.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-03-10
 */
#ifndef PAAL_MAKE_TUPLE_HPP
#define PAAL_MAKE_TUPLE_HPP

namespace paal {
/**
 * @brief function object  for std::make_tuple
 */
struct make_tuple {
    /**
     * @brief operator()
     *
     * @tparam Args
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args) const->decltype(
        std::make_tuple(std::forward<Args>(args)...)) {
        return std::make_tuple(std::forward<Args>(args)...);
    }
};

} //!paal

#endif // PAAL_MAKE_TUPLE_HPP
