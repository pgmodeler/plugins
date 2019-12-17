//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file components_join.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-06-15
 */
#ifndef PAAL_COMPONENTS_JOIN_HPP
#define PAAL_COMPONENTS_JOIN_HPP

#include "paal/data_structures/components/components.hpp"

namespace paal {
namespace data_structures {

namespace detail {

   // assumes that names with defaults are already at the end of
   // concatenation of Components1 and Components2
   template <typename Components1, typename Components2>
      struct concat;

   template <typename... ComponentNamesWithDefaults1, typename... ComponentNamesWithDefaults2>
      struct concat<
         paal::data_structures::components<ComponentNamesWithDefaults1...>,
         paal::data_structures::components<ComponentNamesWithDefaults2...>
      > {
         using type = paal::data_structures::components<ComponentNamesWithDefaults1..., ComponentNamesWithDefaults2...>;
      };

}//!detail

/**
 * @brief Creates new components class with set of names that is the union of
 * names from input components classes. Names are arranged so that all names with
 * defaults are at the end.
 *
 * @tparam Components1
 * @tparam Components2
 */
template <typename Components1, typename Components2>
struct join;

/**
 * @brief First components class has only names with defaults, second components class is empty.
 * This case cannot be simplified to just "Second components class is empty" to disambiguate
 * pattern matching.
 *
 * @tparam Name1
 * @tparam Default1
 * @tparam ComponentNamesWithDefaults1
 */
template <typename Name1, typename Default1, typename... ComponentNamesWithDefaults1>
struct join<components<NameWithDefault<Name1, Default1>, ComponentNamesWithDefaults1...>, components<>> {
   using type = components<NameWithDefault<Name1, Default1>, ComponentNamesWithDefaults1...>;
};

/**
 * @brief Both components classes have only names with defaults.
 *
 * @tparam Name1
 * @tparam Default1
 * @tparam ComponentNamesWithDefaults1
 * @tparam Name2
 * @tparam Default2
 * @tparam ComponentNamesWithDefaults2
 */
template <typename Name1, typename Default1, typename... ComponentNamesWithDefaults1,
   typename Name2, typename Default2, typename... ComponentNamesWithDefaults2>
struct join<components<NameWithDefault<Name1, Default1>, ComponentNamesWithDefaults1...>,
components<NameWithDefault<Name2, Default2>, ComponentNamesWithDefaults2...>> {
   using type = components<
      NameWithDefault<Name1, Default1>,
      ComponentNamesWithDefaults1...,
      NameWithDefault<Name2, Default2>,
      ComponentNamesWithDefaults2...
   >;
};

/**
 * @brief First components class has only names with defaults.
 *
 * @tparam Name1
 * @tparam Default1
 * @tparam ComponentNamesWithDefaults1
 * @tparam ComponentName2
 * @tparam ComponentNamesWithDefaults2
 */
template <typename Name1, typename Default1, typename... ComponentNamesWithDefaults1,
   typename ComponentName2, typename... ComponentNamesWithDefaults2>
struct join<components<NameWithDefault<Name1, Default1>, ComponentNamesWithDefaults1...>,
components<ComponentName2, ComponentNamesWithDefaults2...>> {
   using type = typename detail::concat<
      components<ComponentName2>,
      typename join<
         components<NameWithDefault<Name1, Default1>, ComponentNamesWithDefaults1...>,
         components<ComponentNamesWithDefaults2...>
      >::type
   >::type;
};

/**
 * @brief First components class is empty.
 *
 * @tparam ComponentNamesWithDefaults2
 */
template <typename... ComponentNamesWithDefaults2>
struct join<components<>, components<ComponentNamesWithDefaults2...>> {
   using type = components<ComponentNamesWithDefaults2...>;
};

/**
 * @brief Normal case.
 *
 * @tparam ComponentName1
 * @tparam ComponentNamesWithDefaults1
 * @tparam ComponentNamesWithDefaults2
 */
template <
   typename ComponentName1,
   typename... ComponentNamesWithDefaults1,
   typename... ComponentNamesWithDefaults2
>
struct join<
   components<ComponentName1, ComponentNamesWithDefaults1...>,
   components<ComponentNamesWithDefaults2...>
> {
   using type = typename detail::concat<
      components<ComponentName1>,
      typename join<components<ComponentNamesWithDefaults1...>, components<ComponentNamesWithDefaults2...>>::type
   >::type;
};

} //!data_structures
} //!paal

#endif /* PAAL_COMPONENTS_JOIN_HPP */
