//=======================================================================
// Copyright (c) 2013 Robert Rosolek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file rotate.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-08-20
 */

#ifndef PAAL_ROTATE_HPP
#define PAAL_ROTATE_HPP
#include "paal/utils/type_functions.hpp"

#include <boost/range/iterator.hpp>
#include <boost/range/join.hpp>

#include <iterator>

namespace paal {
namespace utils {

   /**
    * @brief returns rotated view of the given range
    *
    * @tparam ForwardRange
    * @param rng
    * @param n
    *
    * @return
    */
template <class ForwardRange>
auto rotate(const ForwardRange& rng, range_to_diff_type_t<ForwardRange> n)
{
   // TODO for some reason std::next doesn't compile under clang
   // for boost::zip range
   //auto const mid = std::next(std::begin(rng), n);
   auto const mid = std::begin(rng) + n;
   return boost::join(
      boost::make_iterator_range(mid, std::end(rng)),
      boost::make_iterator_range(std::begin(rng), mid)
   );
}

} //!data_structures
} //!paal

#endif // PAAL_ROTATE_HPP
