/**
 * @file maximum_coverage.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-04-16
 */
#ifndef PAAL_MAXIMUM_COVERAGE_HPP
#define PAAL_MAXIMUM_COVERAGE_HPP

#include "paal/greedy/set_cover/budgeted_maximum_coverage.hpp"
#include "paal/utils/functors.hpp"

namespace paal{
namespace greedy{
/**
 * @brief this is solve Set Cover problem
 * and return set cover cost
 * example:
 *  \snippet set_cover_example.cpp Set Cover Example
 *
 * complete example is set_cover_example.cpp
 * @param sets
 * @param set_to_elements
 * @param result set iterators of chosen sets
 * @param get_el_index
 * @param number_of_sets_to_select
 * @param get_weight_of_element
 * @tparam SetRange
 * @tparam GetElementsOfSet
 * @tparam OutputIterator
 * @tparam GetElementIndex
 * @tparam GetWeightOfElement
 */

template<typename SetRange, class GetElementsOfSet, class OutputIterator,class GetElementIndex,
    class GetWeightOfElement=paal::utils::return_one_functor>
auto maximum_coverage(
                SetRange && sets,
                GetElementsOfSet set_to_elements,
                OutputIterator result,
                GetElementIndex get_el_index,
                unsigned int number_of_sets_to_select,
                GetWeightOfElement get_weight_of_element = GetWeightOfElement{}
                ) {
    auto set_to_cost = paal::utils::return_one_functor{};
    return budgeted_maximum_coverage(
        sets,
        set_to_cost,
        set_to_elements,
        result,
        get_el_index,
        number_of_sets_to_select,
        get_weight_of_element,
        0
    );
};
}//!greedy
}//!paal

#endif /* PAAL_MAXIMUM_COVERAGE_HPP */
