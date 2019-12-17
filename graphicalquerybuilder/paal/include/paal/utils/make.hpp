/**
 * @file make.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-11-24
 */
#ifndef PAAL_MAKE_HPP
#define PAAL_MAKE_HPP

#include "paal/utils/type_functions.hpp"

#include <unordered_set>

namespace paal {

/// make for unordered_set
template <typename Range, typename Element = range_to_elem_t<Range>>
auto make_unordered_set(Range const & r) {
    return std::unordered_set<Element>(std::begin(r), std::end(r));
}

} //! paal

#endif /* PAAL_MAKE_HPP */
