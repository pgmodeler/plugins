//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file local_search_obj_function.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-11
 */
#ifndef PAAL_LOCAL_SEARCH_OBJ_FUNCTION_HPP
#define PAAL_LOCAL_SEARCH_OBJ_FUNCTION_HPP

#include "local_search.hpp"
#include "trivial_solution_commit.hpp"
#include "search_obj_func_components.hpp"

#include "paal/data_structures/components/component_traits.hpp"

#include <boost/range/adaptor/transformed.hpp>

namespace paal {
namespace local_search {

/**
 * @brief traits class for search_componentsObjFun
 *
 * @tparam search_componentsObjFun
 */
template <typename SearchComponentsObjFun>
struct search_obj_function_components_traits {
    using get_moves_t = typename data_structures::component_traits<
        SearchComponentsObjFun>::template type<GetMoves>::type;
    using obj_function_t = typename data_structures::component_traits<
        SearchComponentsObjFun>::template type<ObjFunction>::type;
    using commit_t = typename data_structures::component_traits<
        SearchComponentsObjFun>::template type<Commit>::type;
};

namespace detail {

template <typename F, typename GetMoves, typename Commit>
class obj_fun_get_moves {
    F m_f;
    GetMoves const m_get_moves;
    Commit const m_commit;

public:

    obj_fun_get_moves(F f, GetMoves get_moves, Commit commit)
        : m_f(std::move(f)),
          m_get_moves(std::move(get_moves)),
          m_commit(std::move(commit)) {}

    template <typename Solution>
    auto operator()(Solution const &solution) {
        using move = typename move_type_from_get_moves<GetMoves, Solution>::reference;
        return m_get_moves(solution) | boost::adaptors::transformed(
                [&solution, this](move m) {
                        Solution new_solution(solution);
                        m_commit(new_solution, m);
                        return std::make_pair(m, m_f(new_solution));
                    });
    }
};

template <typename Fitness>
class obj_fun_gain {
    Fitness & m_current_res;

public:

    obj_fun_gain(Fitness & current_res)
        : m_current_res(current_res) {}

    template <typename Solution, typename Move>
    auto operator()(Solution const &solution, Move const &move) {
        return move.second - m_current_res;
    }
};

template <typename Commit, typename Fitness>
class obj_fun_commit {
    Commit const m_commit;
    Fitness & m_current_res;

public:

    obj_fun_commit(Commit commit, Fitness & current_res)
        : m_commit(std::move(commit)), m_current_res(current_res) {}

    template <typename Solution, typename Move>
    auto operator()(Solution &solution, Move const &move) {
        if (!m_commit(solution, move.first)) {
            return false;
        }
        m_current_res = move.second;
        return true;
    }
};

template <typename Solution, typename SearchObjFunctionComponents,
          typename Traits = search_obj_function_components_traits<SearchObjFunctionComponents>,
          typename F = typename Traits::obj_function_t,
          typename Fitness = pure_result_of_t<F(Solution &)>>
auto convert_comps(Solution & sol, SearchObjFunctionComponents components, Fitness & current_res) {
    using commit_t = typename Traits::commit_t;
    using get_moves_t = typename Traits::get_moves_t;

    using obj_fun_get_moves = detail::obj_fun_get_moves<F, get_moves_t, commit_t>;
    using obj_fun_gain = detail::obj_fun_gain<Fitness>;
    using obj_fun_commit = detail::obj_fun_commit<commit_t, Fitness>;

    auto get_moves = std::move(components.template get<GetMoves>());
    auto commit = std::move(components.template get<Commit>());
    auto obj_fun = std::move(components.template get<ObjFunction>());

    return make_search_components(
        obj_fun_get_moves(obj_fun, get_moves, commit),
        obj_fun_gain(current_res),
        obj_fun_commit(commit, current_res));
}

} // !detail


///local search function for objective function case.
template <typename SearchStrategy, typename ContinueOnSuccess,
          typename ContinueOnFail, typename Solution,
          typename SearchObjFunctionComponent,
          typename... SearchObjFunctionComponents>
bool local_search_obj_fun(Solution &solution, SearchStrategy searchStrategy,
                          ContinueOnSuccess on_success, ContinueOnFail on_fail,
                          SearchObjFunctionComponent component,
                          SearchObjFunctionComponents ... components) {
    //TODO make it work for many different objective functions
    auto cur_res = component.template call<ObjFunction>(solution);

    return local_search(solution, searchStrategy, std::move(on_success),
                        std::move(on_fail), detail::convert_comps(solution, std::move(component ), cur_res),
                                            detail::convert_comps(solution, std::move(components), cur_res)...);
}

///simple version of local_search_obj_fun - first improving strategy
template <typename Solution, typename... Components>
bool obj_fun_first_improving(Solution &solution, Components... comps) {
    return local_search_obj_fun(solution, first_improving_strategy{},
                                utils::always_true{}, utils::always_false{},
                                std::move(comps)...);
}

///simple version of local_search_obj_fun - best improving strategy
template <typename Solution, typename... Components>
bool obj_fun_best_improving(Solution &solution, Components... comps) {
    return local_search_obj_fun(solution, best_improving_strategy{},
                                utils::always_true{}, utils::always_false{},
                                std::move(comps)...);
}

} // local_search
} // paal

#endif // PAAL_LOCAL_SEARCH_OBJ_FUNCTION_HPP
