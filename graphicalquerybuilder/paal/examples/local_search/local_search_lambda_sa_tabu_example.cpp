//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file local_search_lambda_sa_tabu_example.cpp
 * @brief local search example
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-02-04
 */

    //! [Local Search Example]
#include "paal/local_search/local_search.hpp"
#include "paal/local_search/simulated_annealing.hpp"
#include "paal/data_structures/tabu_list/tabu_list.hpp"
#include "paal/local_search/custom_components.hpp"

#include <vector>
#include <iostream>

int main() {
    namespace ls = paal::local_search;
    using Solution = int;
    using Move = int;

    auto f = [](int x) { return -x * x + 12 * x - 27; };
    // creating solution
    int solution(0);

    // neighborhood
    std::vector<int> neighb{ 10, -10, 1, -1 };

    auto getMoves = [ = ](int) {
        return boost::make_iterator_range(neighb.begin(), neighb.end());
    };

    auto gain = [ = ](int sol, int move) { return f(sol + move) - f(sol); };

    auto commit = [](int & sol, int move) {
        sol = sol + move;
        return true;
    };

    // search
    ls::first_improving(solution,
                        ls::make_search_components(getMoves, gain, commit));

    // print
    std::cout << "Local search solution: " << solution << std::endl;

    // simulated annealing:
    // now each move is accepted with certain probability depending on
    // the move quality and iteration id.
    solution = 0;
    auto cooling = ls::exponential_cooling_schema_dependant_on_iteration(
        1000, 0.999); // this is just a functor returning double
    auto gainSA = ls::make_simulated_annealing_gain_adaptor(
        gain, cooling); // we create new gain by adopting the old one
    ls::first_improving(
        solution, ls::make_search_components(getMoves, gainSA,
                                             commit)); // we run local search

    // print
    std::cout << "Simulated annealing solution: " << solution << std::endl;

    int currentSolution(0);
    int best(0);

    // getMovesRandom returns random move
    std::default_random_engine engine;
    std::uniform_int_distribution<> dist(0, 4);
    auto getMovesRandom = [ = ](int)mutable {
        auto iter = neighb.begin() + dist(engine);
        return boost::make_iterator_range(iter, iter + 1);
    };

    ls::stop_condition_count_limit stop_condition(1000);
    paal::utils::always_false on_fail;

    // this commit remembers the best solution
    // in many cases it should be also used in simulated annealing
    auto recordSolutionCommit = ls::make_record_solution_commit_adapter(
        best, // the reference to the best found solution which is going to be
              // updated during the search
        commit,
        paal::utils::make_functor_to_comparator(
            f)); // recordSolutionCommit must know how to compare solutions

    // random walk
    ls::local_search(currentSolution, ls::first_improving_strategy{},
                     paal::utils::make_not_functor(stop_condition), on_fail,
                     ls::make_search_components(
                         getMovesRandom, paal::utils::return_one_functor(),
                         recordSolutionCommit));

    // print
    std::cout << "Random walk solution: " << best << std::endl;

    // tabu search
    // one of the implementations of tabu list, remembers las 20 (solution,
    // move) pairs.

    currentSolution = 0;
    best = 0;
    auto gainTabu = ls::make_tabu_gain_adaptor(
        paal::data_structures::tabu_list_remember_solution_and_move<
            Move, Solution>(20),
        gain);

    ls::local_search(
        currentSolution, ls::best_improving_strategy{},
        paal::utils::make_not_functor(stop_condition), on_fail,
        ls::make_search_components(getMoves, gainTabu, recordSolutionCommit));

    // print
    std::cout << "Tabu solution: " << best << std::endl;

    return 0;
}
    //! [Local Search Example]
