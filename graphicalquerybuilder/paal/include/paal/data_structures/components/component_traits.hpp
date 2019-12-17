//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file component_traits.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-22
 */
#ifndef PAAL_COMPONENT_TRAITS_HPP
#define PAAL_COMPONENT_TRAITS_HPP
#include "components.hpp"

namespace paal {
namespace data_structures {

template <typename components> struct component_traits;

template <typename Names, typename Types>
struct component_traits<detail::components<Names, Types>> {
    template <typename Name>
    using type = detail::type_for_name<Name, Names, Types>;
};
}
}
#endif // PAAL_COMPONENT_TRAITS_HPP
