//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file paal/local_search/search_components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-18
 */
#ifndef PAAL_SEARCH_COMPONENTS_HPP
#define PAAL_SEARCH_COMPONENTS_HPP

#include "paal/data_structures/components/components.hpp"
#include "paal/utils/functors.hpp"

namespace paal {
namespace local_search {

/**
 * @brief name for the get_moves component
 */
struct GetMoves;
/**
 * @brief name for the Gain component
 */
struct Gain;
/**
 * @brief name for the Commit component
 */
struct Commit;

/**
 * @brief Definition for the components class for local search
 * usually this class is not directly used, see search_components.
 */
typedef data_structures::components<GetMoves, Gain, Commit> components;

/**
 * @brief search_components template alias
 *
 * @tparam Args
 */
template <typename... Args>
using search_components = typename components::type<Args...>;

/**
 * @brief Multisearch_components template alias
 *
 * @tparam Args
 */
template <typename... Args>
using Multisearch_components = search_components<Args...>;

/**
 * @brief make function for search components
 *
 * @tparam Args
 *
 * @return search_components or Multisearch_components
 */
template <typename... Args>
auto make_search_components(Args &&... args) {
    return components::make_components(std::forward<Args>(args)...);
}

} //! local_search
} //! paal
#endif // PAAL_SEARCH_COMPONENTS_HPP
