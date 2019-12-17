//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file fill_knapsack_dynamic_table.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-09-29
 */
#ifndef PAAL_FILL_KNAPSACK_DYNAMIC_TABLE_HPP
#define PAAL_FILL_KNAPSACK_DYNAMIC_TABLE_HPP

#include "paal/utils/knapsack_utils.hpp"

namespace paal {
/**
 * @brief Computes dynamic algorithm table (valuesBegin, valuesEnd)
 *        The values collection has element type ValueOrNull,
 *        The  default constructed ValueOrNull should represent empty object.
 *        This collection is filled using init, compare and combine functors.
 *
 * @param valuesBegin begin of the table which will store
 *  the values for specific positions in dynamic algorithm computation
 * @param valuesEnd
 * @param objects - possible object collection
 * @param size - functor, for given opbjedt return its size
 * @param combine - for given Objects and value gives new object
 * representing adding *Objects to value
 * @param compare - compares to values.
 * @param init - discover element and assign the 0 value
 * @param get_range
 *
 * @tparam ValueIterator has to be RandomAccess output iterator
 * @tparam Objects
 * @tparam ObjectSizeFunctor
 * @tparam Combine
 * @tparam Compare
 * @tparam Init
 * @tparam GetPositionRange
 */
template <typename ValueIterator, typename Objects, typename ObjectSizeFunctor,
          typename Combine, typename Compare, typename Init,
          typename GetPositionRange>
detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>
fill_knapsack_dynamic_table(ValueIterator valuesBegin, ValueIterator valuesEnd,
                            Objects &&objects, ObjectSizeFunctor size,
                            Combine combine, Compare compare, Init init,
                            GetPositionRange get_range) {
    using Size = detail::FunctorOnRangePValue<ObjectSizeFunctor, Objects>;

    Size maxSize = std::distance(valuesBegin, valuesEnd);

    std::fill(valuesBegin + 1, valuesEnd, boost::none);
    init(*valuesBegin);

    auto posRange = get_range(0, maxSize);

    auto objIter = std::begin(objects);
    auto oEnd = std::end(objects);
    for (; objIter != oEnd; ++objIter) {
        auto &&obj = *objIter;
        auto objSize = size(obj);
        // for each position, from largest to smallest
        for (auto pos : posRange) {
            auto stat = *(valuesBegin + pos);
            // if position was reached before
            if (stat != boost::none) {
                Size newPos = pos + objSize;
                auto &newStat = *(valuesBegin + newPos);
                // if we're not exceeding maxSize
                if (newPos < maxSize) {
                    auto newValue = combine(stat, objIter);
                    // if the value is bigger than previous
                    if (newStat == boost::none || compare(newStat, newValue)) {
                        // update value
                        newStat = newValue;
                    }
                }
            }
        }
    }
    return maxSize - 1;
}

}      //! paal
#endif // PAAL_FILL_KNAPSACK_DYNAMIC_TABLE_HPP
