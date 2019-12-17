//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file search_obj_func_components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-18
 */
#ifndef PAAL_SEARCH_OBJ_FUNC_COMPONENTS_HPP
#define PAAL_SEARCH_OBJ_FUNC_COMPONENTS_HPP

#include "paal/data_structures/components/components.hpp"
#include "paal/local_search/search_components.hpp"
#include "paal/utils/functors.hpp"

namespace paal {
namespace local_search {

/**
 * @brief Name for GetMoves component
 */
struct GetMoves;
/**
 * @brief Name for ObjFunction component
 */
struct ObjFunction;
/**
 * @brief Name for Commit component
 */
struct Commit;

/**
 * @brief components for objective function local search.
 *        This usually this class is not used. See search_componentsObjFun
 * class.
 */
using components_obj_fun = data_structures::components<GetMoves, ObjFunction, Commit>;

/**
 * @brief search_componentsObjFun alias to components.
 *
 * @tparam Args
 */
template <typename... Args>
using search_components_obj_fun = typename components_obj_fun::type<Args...>;

/**
 * @brief make function for search_componentsObjFun
 *
 * @tparam Args
 * @param args
 *
 * @return
 */
template <typename... Args>
auto make_search_components_obj_fun(Args &&... args) {
    return components_obj_fun::make_components(std::forward<Args>(args)...);
}

} //! local_search
} //! paal
#endif // PAAL_SEARCH_OBJ_FUNC_COMPONENTS_HPP
