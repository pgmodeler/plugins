//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file voronoi_traits.hpp
 * @brief voronoi traits
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-03-06
 */
#ifndef PAAL_VORONOI_TRAITS_HPP
#define PAAL_VORONOI_TRAITS_HPP

#include "paal/utils/type_functions.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief voronoi traits base
 *
 * @tparam V
 * @tparam Vertex
 */
template <typename V, typename Vertex> struct _voronoi_traits {
    typedef Vertex VertexType;
    /// distance type
    typedef decltype(std::declval<V>().add_generator(
        std::declval<VertexType>())) DistanceType;

    /// Generators set
    typedef puretype(std::declval<V>().get_generators()) GeneratorsSet;

    /// vertices set
    typedef puretype(std::declval<V>().get_vertices()) VerticesSet;
};

/// default VertexType is int.
template <typename V> struct voronoi_traits : public _voronoi_traits<V, int> {};
}
}
#endif // PAAL_VORONOI_TRAITS_HPP
