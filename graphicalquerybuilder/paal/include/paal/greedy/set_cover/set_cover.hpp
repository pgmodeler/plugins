/**
 * @file set_cover.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-02-12
 */
#ifndef PAAL_SET_COVER_HPP
#define PAAL_SET_COVER_HPP

#include "paal/greedy/set_cover/budgeted_maximum_coverage.hpp"
#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/function_output_iterator.hpp>

#include <iterator>
#include <vector>

namespace paal{
namespace greedy{
namespace detail{
template <typename SetRange,typename GetCostOfSet>
using set_range_cost_t = pure_result_of_t<
        GetCostOfSet(typename boost::range_reference<SetRange>::type)>;

}//!detail
/**
 * @brief this is solve Set Cover problem
 * and return set cover cost
 * example:
 *  \snippet set_cover_example.cpp Set Cover Example
 *
 * complete example is set_cover_example.cpp
 * @param sets
 * @param set_to_cost
 * @param set_to_elements
 * @param result set iterators of chosen sets
 * @param get_el_index
 * @tparam SetRange
 * @tparam GetCostOfSet
 * @tparam GetElementsOfSet
 * @tparam OutputIterator
 * @tparam GetElementIndex
 */
template<typename SetRange, class GetCostOfSet, class GetElementsOfSet, class OutputIterator,class GetElementIndex>
auto set_cover(SetRange && sets,
        GetCostOfSet set_to_cost,
        GetElementsOfSet set_to_elements,
        OutputIterator result,
        GetElementIndex get_el_index
    ) {
    using set_cost=typename detail::set_range_cost_t<SetRange,GetCostOfSet>;
    //TODO use sum functor from r=Robert commit
    auto cost_of_all_sets=accumulate_functor(sets, set_cost{}, set_to_cost);
    set_cost cost_of_solution{};
    budgeted_maximum_coverage(sets,
        set_to_cost,
        set_to_elements,
        boost::make_function_output_iterator([&](int set){
            cost_of_solution += set_to_cost(set);
            *result = set;
            ++result;
            }),
        get_el_index,
        cost_of_all_sets,
        paal::utils::return_one_functor(),
        0
    );
    return cost_of_solution;
};
}//!greedy
}//!paal

#endif /* PAAL_SET_COVER_HPP */
