//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file 2_local_search_components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-03-11
 */
#ifndef PAAL_2_LOCAL_SEARCH_COMPONENTS_HPP
#define PAAL_2_LOCAL_SEARCH_COMPONENTS_HPP

#include "paal/data_structures/subset_iterator.hpp"

namespace paal {
namespace local_search {

/**
 * @brief Swap
 *
 * @tparam Element
 */
template <typename Element> struct Swap {
    /**
     * @brief constructor
     *
     * @param from
     * @param to
     */
    Swap(Element from, Element to) : m_from(from), m_to(to) {}

    /**
     * @brief getter for m_from
     *
     * @return
     */
    Element get_from() const { return m_from; }

    /**
     * @brief getter for m_to
     *
     * @return
     */
    Element get_to() const { return m_to; }

  private:
    Element m_from;
    Element m_to;
};

/**
 * @brief Functor creating Move
 */
struct make_swap {
    /**
     * @brief operator()
     *
     * @param from
     * @param to
     *
     * @return
     */
    template <typename Element>
    Swap<Element> operator()(Element from, Element to) const {
        return Swap<Element>(from, to);
    }
};

/**
 * @brief gain for two opt moves
 *
 * @tparam Metric
 */
template <typename Metric> class gain_two_opt {
  public:
    /**
     * @brief
     *
     * @param m fulfills \ref metric concept.
    */
    gain_two_opt(const Metric &m) : m_metric(m) {}

    /**
     * @brief returns gain for given adjustment
     *
     * @tparam Solution
     * @tparam SolutionElement
     * @param swap
     *
     * @return
     */
    template <typename Solution, typename SolutionElement>
    int operator()(const Solution &, const Swap<SolutionElement> &swap) {
        auto from = swap.get_from();
        auto to = swap.get_to();
        return m_metric(from.first, from.second) +
               m_metric(to.first, to.second) - m_metric(from.first, to.first) -
               m_metric(from.second, to.second);
    }

  private:
    const Metric &m_metric;
};

/**
 * @brief Commit class for local_search
 */
struct two_local_search_commit {
    /**
     * @brief flips appropriate segment in the solution
     *
     * @tparam SolutionElement
     * @tparam Solution
     * @param s
     * @param swap
     */
    template <typename SolutionElement, typename Solution>
    bool operator()(Solution &s, const Swap<SolutionElement> &swap) {
        s.get_cycle().flip(swap.get_from().second, swap.get_to().first);
        return true;
    }
};

/**
 * @brief Commit class for local_search
 */
class two_local_searchget_moves {

    /**
     * @brief Functor needed for type computation
     *
     * @tparam Solution
     */
    template <typename Solution> struct types_eval {
        using SolutionIter = decltype(std::declval<Solution>().begin());
        using Subset =
            data_structures::subsets_iterator<2, SolutionIter, make_swap>;
        using Range = boost::iterator_range<Subset>;
    };

  public:
    /**
     * @brief return all pairs of elements from solution
     *
     * @tparam Solution
     * @param solution
     */
    template <typename Solution>
    auto operator()(
        Solution &solution) const->typename types_eval<Solution>::Range {
        return data_structures::make_subsets_iterator_range<2>(
                solution.begin(), solution.end(), make_swap{});
    }
};

} //!local_search
} //!paal

#endif // PAAL_2_LOCAL_SEARCH_COMPONENTS_HPP
