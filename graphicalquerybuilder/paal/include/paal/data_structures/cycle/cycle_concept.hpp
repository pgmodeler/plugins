//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file cycle_concept.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-28
 */
#ifndef PAAL_CYCLE_CONCEPT_HPP
#define PAAL_CYCLE_CONCEPT_HPP

#include "paal/data_structures/cycle/cycle_traits.hpp"

#include <boost/concept_check.hpp>

namespace paal {
namespace data_structures {
namespace concepts {

template <typename X> class Cycle {
  public:
    BOOST_CONCEPT_USAGE(Cycle) {
        ve = x.vbegin();
        ve = x.vbegin(ce);
        ve = x.vend();
        x.flip(ce, ce);
    }

  private:
    X x;
    typename cycle_traits<X>::CycleElem ce;
    typename cycle_traits<X>::vertex_iterator ve;
};
}
}
}

#endif // PAAL_CYCLE_CONCEPT_HPP
