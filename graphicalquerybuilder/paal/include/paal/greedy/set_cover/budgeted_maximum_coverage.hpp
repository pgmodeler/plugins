/**
 * @file budgeted_maximum_coverage.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2014-03-18
 */
#ifndef PAAL_BUDGETED_MAXIMUM_COVERAGE_HPP
#define PAAL_BUDGETED_MAXIMUM_COVERAGE_HPP

#include "paal/data_structures/fraction.hpp"
#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/algorithms/subset_backtrack.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/heap/d_ary_heap.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm/fill.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/combine.hpp>

#include <algorithm>
#include <vector>

namespace paal {
namespace greedy {
namespace detail {

template <typename ElementWeight, typename SetCost> struct set_data_type {
    set_data_type()
        : m_weight_of_uncovered_elements{}, m_cost{}, m_is_processed{ true } {}
    ElementWeight m_weight_of_uncovered_elements; // sum of weight of uncovered
                                                  // elements in backtrack and
                                                  // greedy
    SetCost m_cost;
    bool m_is_processed;
    // set is processed if is selected or is not selected and at least one of
    // the following situations occurs
    // -we have not enough budget to select set
    // -sum of weight of uncovered elements belong to set equal 0
};

template <typename SetReference, typename GetElementsOfSet,
          typename GetWeightOfElement>
using element_weight_t = pure_result_of_t<GetWeightOfElement(
    range_to_elem_t<pure_result_of_t<GetElementsOfSet(SetReference)>>)>;

const int UNCOVERED = -1;
template <class Budget, class SetCost, class SetIdToData, class ElementWeight,
          class SetIdToElements, class ElementIndex, class GetWeightOfElement,
          typename DecreseWeight>
class selector {
    const Budget m_budget;
    SetCost &m_cost_of_solution;
    std::vector<int> m_selected_sets;
    SetIdToData &m_sets_data;
    const ElementWeight &m_weight_of_bests_solution;
    ElementWeight &m_weight_of_covered_elements;
    SetIdToElements m_set_id_to_elements;
    std::vector<int> &m_covered_by;
    std::vector<std::vector<int>> &m_sets_covering_element;
    ElementIndex &m_get_el_index;
    GetWeightOfElement &m_element_to_weight;
    DecreseWeight m_decrese_weight;

    using SetData = range_to_elem_t<SetIdToData>;

  public:
    selector(const Budget budget, SetCost &cost_of_solution,
             SetIdToData &sets_data,
             const ElementWeight &weight_of_bests_solution,
             ElementWeight &weight_of_covered_elements,
             SetIdToElements set_id_to_elements, std::vector<int> &covered_by,
             std::vector<std::vector<int>> &sets_covering_element,
             ElementIndex &get_el_index, GetWeightOfElement &element_to_weight,
             DecreseWeight decrese_weight)
        : m_budget(budget), m_cost_of_solution(cost_of_solution),
          m_sets_data(sets_data),
          m_weight_of_bests_solution(weight_of_bests_solution),
          m_weight_of_covered_elements(weight_of_covered_elements),
          m_set_id_to_elements(set_id_to_elements), m_covered_by(covered_by),
          m_sets_covering_element(sets_covering_element),
          m_get_el_index(get_el_index), m_element_to_weight(element_to_weight),
          m_decrese_weight(decrese_weight) {}

    // we return true if (we select set) or (set is already in solution)
    bool select_set_backtrack(int selected_set_id, bool in_reset = false) {
        if(!can_select(m_sets_data[selected_set_id])) return false;

        select_set(selected_set_id, true);
        if(!in_reset) m_selected_sets.push_back(selected_set_id);
        return true;
    }

    // we return true if we violated budget
    bool select_set_greedy(int selected_set_id) {
        auto &select_set_data = m_sets_data[selected_set_id];

        if(!can_select(select_set_data)) return false;
        m_selected_sets.push_back(selected_set_id);

        if(greedy_prune(select_set_data))  return true;
        select_set(selected_set_id, false);
        return false;
    }


    void deselect_set(int selected_set_id, bool backtrack = true) {
        // we deselect set

        m_cost_of_solution -= m_sets_data[selected_set_id].m_cost;
        for (auto element : m_set_id_to_elements(selected_set_id)) {
            if (covered_by(element) == selected_set_id) {
                m_covered_by[m_get_el_index(element)] = UNCOVERED;
                auto weight_of_element = m_element_to_weight(element);
                cover_element(element, -weight_of_element, backtrack, false);
            }
        }
        m_selected_sets.pop_back();
    }

    void set_unprocessed(
            boost::iterator_range<std::vector<int>::const_iterator> const & set_ids) {
        for (auto set_id : set_ids) {
            m_sets_data[set_id].m_is_processed = false;
        }
    };

    void reset() {
        set_unprocessed(m_selected_sets);
        if (m_selected_sets.size() > 0) {
            boost::for_each(m_selected_sets, [&](int selected_set_id) {
                select_set_backtrack(selected_set_id, true);
            });
        }
    }

    std::size_t size() {
        return m_selected_sets.size();
    }

    void resize(std::size_t size) {
        return m_selected_sets.resize(size);
    }

    template <typename OutputIterator>
    void copy_to(OutputIterator out){
        boost::copy(m_selected_sets, out);
    }
private:

    void select_set(int selected_set_id, bool backtrack) {
        auto &select_set_data = m_sets_data[selected_set_id];

        m_cost_of_solution += select_set_data.m_cost;
        for (auto element : m_set_id_to_elements(selected_set_id)) {
            if (covered_by(element) == UNCOVERED) { /* we do not cover the
                                                        elements being covered*/

                m_covered_by[m_get_el_index(element)] = selected_set_id;
                auto weight_of_element = m_element_to_weight(element);
                cover_element(element, weight_of_element, backtrack);
            }
        }
    }

    /// optimization:
    /// in greedy phase we get sets in decreasing density order, so we can cut
    /// when spend rest of budget with current density product solution, worse
    /// then best found solution.
    bool greedy_prune(const SetData & set_data) {
        return (m_budget - m_cost_of_solution) * set_data.m_weight_of_uncovered_elements <=
                static_cast<Budget>(set_data.m_cost * (m_weight_of_bests_solution - m_weight_of_covered_elements));
    }

    ///this function is ALWAYS called from select_set, thats why we set set_data.m_is_processed!
    bool can_select(SetData & set_data) {
        if (set_data.m_is_processed) return false;
        set_data.m_is_processed = true;
        return static_cast<Budget>(m_cost_of_solution + set_data.m_cost) <= m_budget &&
            set_data.m_weight_of_uncovered_elements!= ElementWeight{};
    }

    template <typename Element>
    void cover_element(Element && el, ElementWeight weight_diff, bool backtrack, bool select = true) {
        m_weight_of_covered_elements += weight_diff;
        for (auto set_id :
                m_sets_covering_element[m_get_el_index(el)]) {
            if (m_sets_data[set_id].m_weight_of_uncovered_elements >
                    weight_diff || backtrack) {
                m_sets_data[set_id].m_weight_of_uncovered_elements -= weight_diff;
                if (!backtrack && !m_sets_data[set_id].m_is_processed) {
                    m_decrese_weight(set_id);
                }
            } else {
                if (select) {
                    m_sets_data[set_id].m_is_processed = true;
                }
            }
        }
    }

    template <typename Element>
    auto covered_by(Element && el) const
        -> decltype(m_covered_by[m_get_el_index(el)]) {

        return m_covered_by[m_get_el_index(el)];
    }
};

template <class Budget, class SetCost, class SetIdToData, class ElementWeight,
         class SetIdToElements, class ElementIndex, class GetWeightOfElement,
         typename DecreseWeight>
auto make_selector(const Budget budget, SetCost &cost_of_solution,
                   SetIdToData &sets_data,
                   const ElementWeight &weight_of_bests_solution,
                   ElementWeight &weight_of_covered_elements,
                   SetIdToElements set_id_to_elements,
                   std::vector<int> &covered_by,
                   std::vector<std::vector<int>> &sets_covering_element,
                   ElementIndex &get_el_index,
                   GetWeightOfElement &element_to_weight,
                   DecreseWeight decrese_weight) {
    return selector<Budget, SetCost, SetIdToData, ElementWeight,
                    SetIdToElements, ElementIndex, GetWeightOfElement,
                    DecreseWeight>(
        budget, cost_of_solution, sets_data,
        weight_of_bests_solution, weight_of_covered_elements,
        set_id_to_elements, covered_by, sets_covering_element, get_el_index,
        element_to_weight, decrese_weight);
}
} //!detail

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
 * @param budget
 * @param element_to_weight
 * @param initial_set_size
 * @tparam SetRange
 * @tparam GetCostOfSet
 * @tparam GetElementsOfSet
 * @tparam OutputIterator
 * @tparam ElementIndex
 * @tparam Budget
 * @tparam GetWeightOfElement
 */
template <typename SetRange, class GetCostOfSet, class GetElementsOfSet,
          class OutputIterator, class ElementIndex, class Budget,
          class GetWeightOfElement = utils::return_one_functor>
auto budgeted_maximum_coverage(
    SetRange && sets, GetCostOfSet set_to_cost,
    GetElementsOfSet set_to_elements, OutputIterator result,
    ElementIndex get_el_index, Budget budget,
    GetWeightOfElement element_to_weight = GetWeightOfElement(),
    const unsigned int initial_set_size = 3) {

    using set_reference = typename boost::range_reference<SetRange>::type;
    using element_weight = typename detail::element_weight_t<
        set_reference, GetElementsOfSet, GetWeightOfElement>;
    using set_cost = pure_result_of_t<GetCostOfSet(set_reference)>;
    using set_id_to_data = std::vector<detail::set_data_type<element_weight, set_cost>>;

    auto nu_sets = boost::distance(sets);
    set_id_to_data initial_sets_data(nu_sets), sets_data(nu_sets);
    set_cost cost_of_solution{}, cost_of_best_solution{};
    int number_of_elements = 0;

    // we find max index of elements in all sets
    for (auto set_and_data : boost::combine(sets, initial_sets_data)) {
        auto const & set = boost::get<0>(set_and_data);
        auto &cost = boost::get<1>(set_and_data).m_cost;
        cost = set_to_cost(set);
        assert(cost != set_cost{});
        auto const & elements = set_to_elements(set);
        if (!boost::empty(elements)) {
            number_of_elements = std::max(number_of_elements,
                    *max_element_functor(elements, get_el_index) + 1);
        }
    }
    element_weight weight_of_covered_elements{}, weight_of_bests_solution{};
    std::vector<int> best_solution(1);
    std::vector<int> covered_by(
        number_of_elements, detail::UNCOVERED); // index of the first set that covers
                                        // element or -1 if element is uncovered
    std::vector<int> sets_id(nu_sets);
    boost::iota(sets_id, 0);
    std::vector<std::vector<int>> sets_covering_element(number_of_elements);
    auto decreasing_density_order =
        utils::make_functor_to_comparator([&](int x) {
            return data_structures::make_fraction(
                sets_data[x].m_weight_of_uncovered_elements, sets_data[x].m_cost);
        });
    using queue = boost::heap::d_ary_heap<
        int, boost::heap::arity<3>, boost::heap::mutable_<true>,
        boost::heap::compare<decltype(decreasing_density_order)>>;

    queue uncovered_set_queue{ decreasing_density_order };
    std::vector<typename queue::handle_type> set_id_to_handle(nu_sets);

    // we fill sets_covering_element and setToWeightOfElements
    for (auto set : sets | boost::adaptors::indexed()) {
        auto set_id = set.index();
        auto &set_data = initial_sets_data[set_id];

        for (auto &&element : set_to_elements(set.value())) {
            sets_covering_element[get_el_index(element)].push_back(set_id);
            set_data.m_weight_of_uncovered_elements += element_to_weight(element);
        }
        if (initial_set_size ==
            0) { /* we check all one element set. if initial_set_size!= 0 then
                    we will do it anyway */
            set_cost cost_of_set = set_data.m_cost;
            if (set_data.m_weight_of_uncovered_elements >= weight_of_bests_solution &&
                static_cast<Budget>(cost_of_set) <= budget) {
                weight_of_bests_solution = set_data.m_weight_of_uncovered_elements;
                best_solution[0] = set_id;
                cost_of_best_solution = cost_of_set;
            }
        }
    };

    auto sort_sets = [&](std::vector<int>& sets_range) {
        boost::sort(sets_range, utils::make_functor_to_comparator([&](int x) {
                                    return sets_data[x].m_weight_of_uncovered_elements;
                                }));
        return sets_range.end();
    };
    auto selector = detail::make_selector
                (budget, cost_of_solution,sets_data,
                 weight_of_bests_solution,weight_of_covered_elements,
                 [&](int selected_set_id){return set_to_elements(sets[selected_set_id]);},
                 covered_by,sets_covering_element,get_el_index,
                 element_to_weight,
                 [&](int set_id){uncovered_set_queue.decrease(set_id_to_handle[set_id]);});

    boost::copy(initial_sets_data, sets_data.begin());
    sort_sets(sets_id);
    auto solver = make_subset_backtrack(sets_id);

    auto reset = [&]() {
        boost::copy(initial_sets_data, sets_data.begin());
        cost_of_solution = set_cost{};
        selector.set_unprocessed(solver.get_moves());
        boost::fill(covered_by, detail::UNCOVERED);
        weight_of_covered_elements = element_weight{};
        selector.reset();
    };

    auto on_pop = [&](int deselected_set_id) {
        selector.deselect_set(deselected_set_id);
        selector.set_unprocessed(solver.get_moves());
    };

    auto save_best_solution = [&]() {
        // we check that new solution is better than any previous
        // if is we remember them
        // tricky: either better weight, or equal weight and lower cost
        if (std::make_pair(weight_of_covered_elements, cost_of_best_solution) >
            std::make_pair(weight_of_bests_solution, cost_of_solution)) {
            weight_of_bests_solution = weight_of_covered_elements;
            cost_of_best_solution = cost_of_solution;
            best_solution.resize(selector.size());
            selector.copy_to(best_solution.begin());
        }
    };
    auto greedy_phase = [&]() {
        uncovered_set_queue.clear();
        auto moves = solver.get_moves();
        if(boost::empty(moves)) return;

        for (auto set_id : moves) {
            set_id_to_handle[set_id] = uncovered_set_queue.push(set_id);
        }
        /* we select set with best elements to cost ratio, and add it to the
         * result until all elements are covered*/
        int uncovered_set_id;
        do {
            uncovered_set_id = uncovered_set_queue.top();
            uncovered_set_queue.pop();
        } while (!selector.select_set_greedy(uncovered_set_id) &&
                !uncovered_set_queue.empty());
    };

    auto can_push = [&](int candidate) {
        if (!selector.select_set_backtrack(candidate)) {
            return false;
        }
        if (selector.size() == initial_set_size) {
            greedy_phase();
            save_best_solution();
            selector.resize(initial_set_size-1);
            reset();
            return false;
        } else {
            save_best_solution();
            return true;
        }
    };

    reset();
    if (initial_set_size != 0) { /* if initial_set_size == 0 then we do greedy
    algorithm once starts from empty initial Set.
    Otherwise we starts greedy algorithm from all initial set of size equal
    initial_set_size those for which we have enough budget*/
        solver.solve(can_push, on_pop, sort_sets);
    } else {
        greedy_phase();
        save_best_solution();
    }

    for (auto set_id : best_solution) {
        *result = *(sets.begin() + set_id);
        ++result;
    }
    return weight_of_bests_solution;
};
} //!greedy
} //!paal

#endif /* PAAL_BUDGETED_MAXIMUM_COVERAGE_HPP */
