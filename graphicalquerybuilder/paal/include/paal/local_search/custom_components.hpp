//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file custom_components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-26
 */
#ifndef PAAL_CUSTOM_COMPONENTS_HPP
#define PAAL_CUSTOM_COMPONENTS_HPP

#include "paal/utils/functors.hpp"

#include "paal/local_search/local_search.hpp"

#include <chrono>
#include <random>

namespace paal {
namespace local_search {

/**
 * @brief if the  condition is not fulfilled this gain adaptor returns 0
 *
 * @tparam Gain
 * @tparam Condition
 */
template <typename Gain = utils::return_one_functor,
          typename Condition = utils::always_true>
struct conditional_gain_adaptor {

    /**
     * @brief constructor
     *
     * @param gain
     * @param cond
     */
    conditional_gain_adaptor(Gain gain = Gain(), Condition cond = Condition())
        : m_gain(std::move(gain)), m_condition(std::move(cond)) {}

    /**
     * @brief
     *
     * @tparam Args to be forwarded
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args)
        ->decltype(std::declval<Gain>()(std::forward<Args>(args)...)) {
        if (!m_condition(std::forward<Args>(args)...)) {
            return 0;
        }
        return m_gain(std::forward<Args>(args)...);
    }

  private:
    Gain m_gain;
    Condition m_condition;
};

/**
 * @brief make for conditional_gain_adaptor
 *
 * @tparam Gain
 * @tparam Condition
 * @param gain
 * @param cond
 *
 * @return
 */
template <typename Gain = utils::return_one_functor,
          typename Condition = utils::always_true>
conditional_gain_adaptor<Gain, Condition>
make_conditional_gain_adaptor(Gain gain = Gain(),
                              Condition cond = Condition()) {
    return conditional_gain_adaptor<Gain, Condition>(std::move(gain),
                                                     std::move(cond));
}

/**
 * @brief This is the gain adapter which accepts gains improving the current
* solution by more than epsilon.
 *        This adapter should be used only when ChooseFirstBetter strategy is
* applied.
 *
 * @tparam Gain
 * @tparam ValueType
 */
template <typename Gain, typename ValueType> class gain_cut_small_improves {
  public:
    /**
     * @brief Constructor,
     *
     * @param gain - original gain functor
     * @param currOpt - current optimum
     * @param epsilon - gain limit, gains smaller than epsilon * currOpt are cut
     */
    gain_cut_small_improves(Gain gain, ValueType currOpt, double epsilon)
        : m_gain(std::move(gain)), m_curr_opt(currOpt), m_epsilon(epsilon) {}

    /**
     * @brief transfers arguments to original gain, if the value is to small it
    * is changed to 0.
     *
     * @tparam Args
     * @param args
     *
     * @return
     */
    template <typename... Args> ValueType operator()(Args &&... args) {
        ValueType dist = m_gain(std::forward<Args>(args)...);
        if (dist > m_epsilon * m_curr_opt) {
            m_curr_opt -= dist;
            return dist;
        }
        return 0;
    }

    /**
     * @brief sets epsilon
     *
     * @param e
     */
    void set_epsilon(double e) { m_epsilon = e; }

    /**
     * @brief sets current Opt
     *
     * @param opt
     */
    void set_current_opt(ValueType opt) { m_curr_opt = opt; }

  private:
    Gain m_gain;
    ValueType m_curr_opt;
    double m_epsilon;
};

/**
 * @brief This is custom StopCondition , it returns true after given count limit
 */
class stop_condition_count_limit {
  public:
    /**
     * @brief Constructor
     *
     * @param limit given count limit
     */
    stop_condition_count_limit(unsigned limit) : m_cnt(0), m_limit(limit) {}

    /**
     * @brief increment the counter and checks if the given limit is reached.
     *
     * @tparam Args
     *
     * @return
     */
    template <typename... Args> bool operator()(Args &&...) const {
        return ++m_cnt >= m_limit;
    }

  private:
    mutable unsigned m_cnt;
    const unsigned m_limit;
};

/**
 * @brief This is custom StopCondition, it returns true after given time limit
 */
template <typename duration = std::chrono::seconds,
          typename clock = std::chrono::system_clock>
class stop_condition_time_limit {
  public:
    /**
     * @brief Constructor
     *
     * @param d - time to wait
     */
    stop_condition_time_limit(duration d)
        : m_duration(d), m_start(clock::now()) {}

    /**
     * @brief Checks if the time is up
     *
     * @tparam Args
     * @param ...
     *
     * @return true if the time is up
     */
    template <typename... Args> bool operator()(Args &&...) {
        return m_start + m_duration < clock::now();
    }

    /**
     * @brief resets the start point
     */
    void restart() { m_start = clock::now(); }

  private:
    typename clock::time_point m_start;
    const duration m_duration;
};

/**
 * @brief This wrapper counts sum of the improvements.
 * It makes sense to use it only when ChooseFirstBetter strategy is applied.
 *
 *
 * @tparam Gain
 * @tparam ValueType
 */
template <typename Gain, typename ValueType> struct compute_gain_wrapper {
    /**
     * @brief Constructor
     *
     * @param gain
     * @param val
     */
    compute_gain_wrapper(compute_gain_wrapper gain, ValueType &val)
        : m_gain(gain), m_val(val) {}

    /**
     * @brief forwards args to original gain. Sum up the improvements.
     *
     * @tparam Args
     * @param args
     *
     * @return
     */
    template <typename... Args> ValueType operator()(Args &&... args) {
        auto diff = m_gain(std::forward<Args>(args)...);
        m_val += diff;
        return diff;
    }

    /**
     * @brief Returns sum of the improvements
     *
     * @return
     */
    ValueType get_val() const { return m_val; }

  private:
    Gain m_gain;
    ValueType &m_val;
};

/**
 * @brief Adapts gain to implement tabu search
 *
 * @tparam TabuList
 * @tparam Gain
 * @tparam AspirationCriteria
 */
template <typename TabuList, typename Gain = utils::return_one_functor,
          typename AspirationCriteria = utils::always_true>
struct tabu_gain_adaptor {

    /**
     * @brief constructor
     *
     * @param tabuList
     * @param gain
     * @param aspirationCriteria
     */
    tabu_gain_adaptor(TabuList tabuList = TabuList(), Gain gain = Gain(),
                      AspirationCriteria aspirationCriteria =
                          AspirationCriteria())
        : m_tabu_list(std::move(tabuList)),
          m_aspiration_criteria_gain(std::move(gain),
                                     std::move(aspirationCriteria)) {}

    /**
     * @brief operator()
     *
     * @tparam Args args to be forwarded
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args)
        ->decltype(std::declval<Gain>()(std::forward<Args>(args)...)) {
        if (!m_tabu_list.is_tabu(std::forward<Args>(args)...)) {
            auto diff = m_aspiration_criteria_gain(std::forward<Args>(args)...);
            if (detail::positive_delta(diff)) {
                m_tabu_list.accept(std::forward<Args>(args)...);
            }
            return diff;
        }
        return decltype(std::declval<Gain>()(std::forward<Args>(args)...)){};
    }

  private:
    TabuList m_tabu_list;
    conditional_gain_adaptor<Gain, AspirationCriteria>
        m_aspiration_criteria_gain;
};

/**
 * @brief make function for tabu_gain_adaptor
 *
 * @tparam TabuList
 * @tparam Gain
 * @tparam AspirationCriteria
 * @param tabuList
 * @param gain
 * @param aspirationCriteria
 *
 * @return
 */
template <typename TabuList, typename Gain = utils::always_true,
          typename AspirationCriteria = utils::always_true>
tabu_gain_adaptor<TabuList, Gain, AspirationCriteria>
make_tabu_gain_adaptor(TabuList tabuList, Gain gain = Gain(),
                       AspirationCriteria aspirationCriteria =
                           AspirationCriteria()) {
    return tabu_gain_adaptor<TabuList, Gain, AspirationCriteria>(
        std::move(tabuList), std::move(gain), std::move(aspirationCriteria));
}

/**
 * @brief This is adaptor on Commit which allows to record solution basing on
* condition
 *        It is particularly useful in tabu search  and simulated annealing
 *        in which we'd like to store the best found solution
 *
 * @tparam Commit
 * @tparam Solution
 * @tparam Comparator
 */
template <typename Commit, typename Solution, typename Comparator = utils::less>
struct record_solution_commit_adapter {

    /**
     * @brief constructor
     *
     * @param solution
     * @param commit
     * @param comparator
     */
    record_solution_commit_adapter(Solution &solution, Commit commit = Commit{},
                                   Comparator comparator = Comparator{})
        : m_solution(&solution), m_commit(std::move(commit)),
          m_comparator(std::move(comparator)) {}

    /**
     * @brief operator
     *
     * @tparam Move
     * @param sol
     * @param move
     */
    template <typename Move> bool operator()(Solution &sol, const Move &move) {
        auto ret = m_commit(sol, move);
        if (m_comparator(*m_solution, sol)) {
            *m_solution = sol;
        }
        return ret;
    }

    /**
     * @brief Access to the stored solution (const version)
     *
     * @return
     */
    const Solution &get_solution() const { return *m_solution; }

    /**
     * @brief Access to the stored solution (non-const version)
     *
     * @return
     */
    Solution &get_solution() { return *m_solution; }

  private:
    Solution *m_solution;
    Commit m_commit;
    Comparator m_comparator;
};

/**
 * @brief make function for record_solution_commit_adapter
 *
 * @tparam Commit
 * @tparam Solution
 * @tparam Condition
 * @param s
 * @param commit
 * @param c
 *
 * @return
 */
template <typename Commit, typename Solution, typename Condition>
record_solution_commit_adapter<Commit, Solution, Condition>
make_record_solution_commit_adapter(Solution &s, Commit commit, Condition c) {
    return record_solution_commit_adapter<Commit, Solution, Condition>(
        s, std::move(commit), std::move(c));
}

} // local_search
} // paal

#endif // PAAL_CUSTOM_COMPONENTS_HPP
