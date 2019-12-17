//=======================================================================
// Copyright (c) 2013 Robert Rosolek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file concepts.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-07-23
 */
#ifndef PAAL_CONCEPTS_HPP
#define PAAL_CONCEPTS_HPP

#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>
#include <boost/range/iterator.hpp>

#include <type_traits>
#include <utility>

namespace paal {
namespace utils {
/// Utils concepts namespace
namespace concepts {

/**
 * @brief Concept class for floating point types.
 *
 * @tparam T
 */
template <class T>
struct floating_point {
   static_assert(std::is_floating_point<T>::value,
         "T is not floating point!");
};

/**
 * @brief Concept class for move constructible types.
 *
 * @tparam T
 */
template <class T>
struct move_constructible {
   static_assert(std::is_move_constructible<T>::value,
         "T is not move constructible!");
};

/**
 * @brief Concept class for output iterator concept.
 * We don't use boost::OutputIterator concept as it excludes
 * boost::function_output_iterator with lambda function.
 *
 * @tparam Iter
 * @tparam Val
 */
template <class Iter, class Val>
class output_iterator {
   Iter iter;
   Val val;

   public:
      BOOST_CONCEPT_USAGE(output_iterator)
      {
         *iter = std::move(val);
         ++iter;
      }
};

/**
 * @brief Concept class for readable range concept.
 *
 * @tparam T
 */
template<class T>
struct readable_range {
   BOOST_CONCEPT_ASSERT((boost_concepts::ReadableIteratorConcept<
      typename boost::range_iterator<T>::type>));
};


} //!concepts
} //!utils
} //!paal

#endif /* PAAL_CONCEPTS_HPP */
