//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file cycle_traits.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-26
 */
#ifndef PAAL_CYCLE_TRAITS_HPP
#define PAAL_CYCLE_TRAITS_HPP

#include "paal/utils/type_functions.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief traits for \ref cycle concept
 *
 * @tparam Cycle
 */
template <typename Cycle> struct cycle_traits {
    /// Vertex iterator type
    typedef decltype(std::declval<Cycle>().vbegin()) vertex_iterator;
    typedef typename std::iterator_traits<vertex_iterator>::value_type
        CycleElem;
};
}
}
#endif // PAAL_CYCLE_TRAITS_HPP
