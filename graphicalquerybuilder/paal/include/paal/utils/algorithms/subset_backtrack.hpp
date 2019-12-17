/**
 * @file subset_backtrack.hpp
 * @brief
 * @author Piotr Smulewicz, Robert Rosołek
 * @version 1.0
 * @date 2014-08-13
 */
#ifndef PAAL_SUBSET_BACKTRACK_HPP
#define PAAL_SUBSET_BACKTRACK_HPP

#include "paal/data_structures/stack.hpp"
#include "paal/utils/concepts.hpp"
#include "paal/utils/singleton_iterator.hpp"

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/join.hpp>

#include <vector>

namespace paal {
namespace detail {

template <typename Element> struct tail {
    tail() = default;
    tail(const tail &that) = delete;
    tail(tail &&other) = default;
    std::vector<Element> m_elems;
    typename std::vector<Element>::const_iterator m_cur_pos;
};

} //! detail

/**
 * @brief we go through all subsets of the input
 */
template <typename Element> class subset_backtrack {
     BOOST_CONCEPT_ASSERT((boost::CopyConstructibleConcept<Element>));
     BOOST_CONCEPT_ASSERT((boost::DefaultConstructible<Element>));
    const std::size_t INITIAL_TAILS_SIZE = 1;
    data_structures::stack<detail::tail<Element>> m_tails;
    bool m_start_solving =false;
  public:
    /// constructor
    template <class Elements>
    subset_backtrack(const Elements &elements)
        : m_tails() {
        BOOST_CONCEPT_ASSERT((utils::concepts::readable_range<Elements>));
        push(elements);
    }

    /**
    * @brief return range of all Elements that we will be trying to add to the
    * current set
    */
    boost::iterator_range<typename std::vector<Element>::const_iterator>
    get_moves() const {
        auto const moves_begin = current_tail().m_cur_pos + m_start_solving,
                   moves_end = current_tail().m_elems.cend();
        return boost::make_iterator_range(moves_begin, moves_end);
    };

    /**
     * @param try_push try add element to solution. return true iff it is
     * successful.
     * @param on_pop
     * @param update_moves This function allows user to change the order of moves.
        User can also remove moves.
        User gets range of possible elements and returns an iterator to the
        element that follows the last element not removed
     */
    template <typename TryPush, typename OnPop,
              typename UpdateMoves = std::mem_fun_ref_t<
                  typename std::vector<Element>::iterator, std::vector<int>>>
    void
    solve(TryPush try_push, OnPop on_pop,
          UpdateMoves update_moves = UpdateMoves(&std::vector<Element>::end)) {
        m_start_solving=true;
        assert(m_tails.size() == INITIAL_TAILS_SIZE);
        if (current_tail().m_elems.empty()) return;
        auto go_down = [&]() {
            while (try_push(current_element())) {
                auto &moves = get_new_tail();
                moves.erase(update_moves(moves), moves.end());
                if (moves.empty()) {
                    m_tails.pop();
                    on_pop(current_element());
                    break;
                }
            };
        };
        go_down();
        while (true) {
            while (try_increase_back()) go_down();
            if (m_tails.size() == INITIAL_TAILS_SIZE) break;
            m_tails.pop();
            on_pop(current_element());
        }
        current_tail().m_cur_pos = current_tail().m_elems.begin();
        m_start_solving=false;
    }

  private:
    std::vector<Element> &get_new_tail() {
        push(get_moves());
        return current_tail().m_elems;
    }

    detail::tail<Element> &current_tail() { return m_tails.top(); }

    const detail::tail<Element> &current_tail() const { return m_tails.top(); }

    Element current_element() const { return *current_tail().m_cur_pos; }

    bool try_increase_back() {
        return ++current_tail().m_cur_pos != current_tail().m_elems.end();
    }

    template <typename Elements> void push(Elements &&elements) {
        m_tails.push();
        auto &new_tail = m_tails.top();
        auto &moves = new_tail.m_elems;
        moves.resize(boost::distance(elements));
        boost::copy(elements, moves.begin());
        new_tail.m_cur_pos = moves.begin();
    };
};


/// make subset_backtrack
template <class Elements>
auto make_subset_backtrack(const Elements &elements) {
    return subset_backtrack<typename boost::range_value<Elements>::type>(
        elements);
}

} //! paal

#endif /* PAAL_SUBSET_BACKTRACK_HPP */
