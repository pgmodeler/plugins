//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file local_search.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_LOCAL_SEARCH_HPP
#define PAAL_LOCAL_SEARCH_HPP

#include "local_search_concepts.hpp"

#include "paal/utils/fusion_algorithms.hpp"
#include "paal/utils/infinity.hpp"
#include "paal/data_structures/components/component_traits.hpp"

#include <boost/fusion/include/vector.hpp>
#include <boost/range/algorithm/max_element.hpp>

#include <utility>
#include <algorithm>
#include <functional>

namespace paal {
namespace local_search {

namespace detail {
    template <typename Delta>
    bool positive_delta(Delta && d) {
        return d > typename std::decay<Delta>::type{};
    }
}

/**
 * @brief this predicates returns true if there is a move with positive gain
 *        and the commit was successful
 */
struct find_positive_predicate {

    /**
     * @brief operator()
     *
     * @tparam componentsAndSolution
     * @param compsAndSol
     *
     * @return true after successful commit, false otherwise
     */
    template <typename componentsAndSolution>
    bool operator()(componentsAndSolution &compsAndSol) const {
        auto &solution = compsAndSol.second;
        auto &comps = compsAndSol.first;
        auto && adjustmentSet = comps.template call<GetMoves>(solution);

        for (auto && move : adjustmentSet) {
            if (detail::positive_delta(comps.template call<Gain>(solution, move))) {
                if (comps.template call<Commit>(solution, move)) {
                    return true;
                }
            }
        }

        return false;
    }
};

/**
 * @brief This strategy uses find_positive_predicate as stop condition
 */
struct first_improving_strategy {
    /**
     * @brief operator()
     *
     * @tparam SearchJoin
     * @param join
     *
     * @return
     */
    template <typename SearchJoin> bool operator()(SearchJoin &join) const {
        return m_satisfy(m_pred, join);
    }

  private:
    find_positive_predicate m_pred;
    data_structures::Satisfy m_satisfy;
};

/**
 * @brief functor used in fold in order to find the most
 * improving move.
 */
struct max_functor {

    /**
     * @brief operator()
     *
     * @tparam componentsAndSolution
     * @tparam AccumulatorFunctor
     * @tparam AccumulatorData
     * @tparam Continuation
     * @param compsAndSol
     * @param accumulatorFunctor
     * @param accumulatorData
     * @param continuation
     *
     * @return
     */
    template <typename componentsAndSolution, typename AccumulatorFunctor,
              typename AccumulatorData, typename Continuation>
    bool operator()(componentsAndSolution &compsAndSol,
                    AccumulatorFunctor accumulatorFunctor,
                    AccumulatorData accumulatorData,
                    Continuation continuation) const {
        auto &comps = compsAndSol.first;
        auto &solution = compsAndSol.second;
        auto && adjustmentSet = comps.template call<GetMoves>(solution);

        //thanks to this checks move doesn't have to be default constructible
        if (boost::empty(adjustmentSet)) {
            return continuation(accumulatorFunctor, accumulatorData);
        }

        auto maxMove = *std::begin(adjustmentSet);
        auto maxGain = comps.template call<Gain>(solution, maxMove);

        for (auto && move : boost::make_iterator_range(
                 ++std::begin(adjustmentSet), std::end(adjustmentSet))) {
            auto gain = comps.template call<Gain>(solution, move);
            if (gain > maxGain) {
                maxMove = move;
                maxGain = gain;
            }
        }

        if (maxGain > accumulatorData) {
            auto commit = std::bind(std::ref(comps.template get<Commit>()),
                                    std::ref(solution), maxMove);
            return continuation(commit, maxGain);
        } else {
            return continuation(accumulatorFunctor, accumulatorData);
        }
    }
};

/**
 * @brief This strategy chooses the best possible move and if it is improving
 * applies it to the solution
 */
struct best_improving_strategy {
    /**
     * @brief operator()
     *
     * @tparam SearchJoin
     * @param join
     *
     * @return
     */
    template <typename SearchJoin> bool operator()(SearchJoin &join) const {
        return m_fold(m_fun, utils::always_false{}, 0, join);
    }

  private:
    max_functor m_fun;
    data_structures::polymorfic_fold m_fold;
};

/**
 * @brief This strategy chooses the best possible move and applies it to the
 * solution.
 *        Note that this strategy might commit non-improving moves
 */
struct best_strategy {
    /**
     * @brief operator()
     *
     * @tparam SearchJoin
     * @param join
     *
     * @return
     */
    template <typename SearchJoin> bool operator()(SearchJoin &join) const {
        return m_fold(m_fun, utils::always_false{}, minus_infinity{}, join);
    }

  private:
    max_functor m_fun;
    data_structures::polymorfic_fold m_fold;
};

namespace detail {

template <typename Solution, typename... SearchComponentsPack>
struct local_search_concepts;

template <typename Solution, typename SearchComponents,
          typename... SearchComponentsPack>
struct local_search_concepts<
    Solution, SearchComponents,
    SearchComponentsPack...> : public local_search_concepts<
    Solution, SearchComponentsPack...> {
    BOOST_CONCEPT_ASSERT(
        (concepts::search_components<SearchComponents, Solution>));
};

template <typename Solution> struct local_search_concepts<Solution> {};

} //!detail

/**
 * @brief local search simple solution
 *
 * @param solution the initial solution which going to be possibly improved by
 *local_search
 * @param searchStrategy
 * @param succ post search action
 * @param fail global stop condition
 * @param comps
 *
 * @return true if the solution is improved
 */
template <typename SearchStrategy, typename ContinueOnSuccess,
          typename ContinueOnFail, typename Solution, typename... components>
bool local_search(Solution &solution, SearchStrategy searchStrategy,
                  ContinueOnSuccess succ, ContinueOnFail fail,
                  components... comps) {
    detail::local_search_concepts<Solution, components...> concepts;
    boost::ignore_unused_variable_warning(concepts);

    using search_components_v =
        boost::fusion::vector<std::pair<components, Solution &>...>;

    search_components_v search_comps(
        std::pair<components, Solution &>(std::move(comps), solution)...);

    bool success{ false }, ret{ false };

    while ((success = searchStrategy(search_comps)) || fail(solution)) {
        ret |= success;
        if (success && !succ(solution)) {
            break;
        }
    }
    return success | ret;
}

/**
 * @brief
 *
 * @param solution the initial solution which going to be possibly improved by
 *local_search
 * @param comps
 *
 * @return true if the solution is improved
 */
template <typename Solution, typename... components>
bool first_improving(Solution &solution, components... comps) {
    return local_search(solution, first_improving_strategy{},
                        utils::always_true{}, utils::always_false{},
                        std::move(comps)...);
}

/**
 * @brief This local search chooses the best possible move and if the move is
* improving applies it to the solution.
 *
 * @tparam Solution
 * @tparam components
 * @param solution
 * @param comps
 *
 * @return
 */
template <typename Solution, typename... components>
bool best_improving(Solution &solution, components... comps) {
    return local_search(solution, best_improving_strategy{},
                        utils::always_true{}, utils::always_false{},
                        std::move(comps)...);
}

/**
 * @brief This local search chooses the best possible move and applies it to the
* solution.
 *        Note that this strategy might commit non-improving moves
 *
 * @tparam Solution
 * @tparam ContinueOnSuccess
 * @tparam components
 * @param solution
 * @param on_success
 * @param comps
 *
 * @return
 */
template <typename Solution, typename ContinueOnSuccess, typename... components>
bool best(Solution &solution, ContinueOnSuccess on_success,
          components... comps) {
    return local_search(solution, best_strategy{}, std::move(on_success),
                        utils::always_false{}, std::move(comps)...);
}

} // local_search
} // paal

#endif // PAAL_LOCAL_SEARCH_HPP
