//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file iterative_rounding_example.cpp
 * @brief Iterative rounding example
 * This is an example implementation of an algorithm within the Iterative
 * Rounding framework.
 * The implemented algorithm is a vertex cover 2-approximation.
 * @author Piotr Godlewski
 * @version 1.0
 * @date 2014-03-24
 */


//! [Iterative Rounding Problem Example]
#include "paal/iterative_rounding/iterative_rounding.hpp"
#include "paal/utils/floating.hpp"
#include "paal/utils/functors.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <iostream>
#include <unordered_map>

namespace ir = paal::ir;

template <typename Graph, typename CostMap, typename OutputIter>
class vertex_cover {
public:
    vertex_cover(const Graph & g, CostMap cost_map, OutputIter cover) :
        m_g(g), m_cost_map(cost_map), m_cover(cover) {}

    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using VertexMap = std::unordered_map<Vertex, paal::lp::col_id>;

    const Graph &get_graph() const { return m_g; }

    auto get_cost(Vertex v)->decltype(std::declval<CostMap>()(v)) {
        return m_cost_map(v);
    }

    void bind_col_to_vertex(paal::lp::col_id col, Vertex v) {
        m_vertex_map.insert(typename VertexMap::value_type(v, col));
    }

    paal::lp::col_id vertex_to_column(Vertex v) { return m_vertex_map[v]; }

    void add_to_cover(Vertex v) {
        *m_cover = v;
        ++m_cover;
    }

  private:
    const Graph &m_g;
    CostMap m_cost_map;
    OutputIter m_cover;
    VertexMap m_vertex_map;
};
//! [Iterative Rounding Problem Example]

//! [Iterative Rounding Components Example]
struct vertex_cover_init {
    template <typename Problem, typename LP>
    void operator()(Problem &problem, LP &lp) {
        lp.set_optimization_type(paal::lp::MINIMIZE);

        // variables for vertices
        for (auto v :
             boost::make_iterator_range(vertices(problem.get_graph()))) {
            problem.bind_col_to_vertex(lp.add_column(problem.get_cost(v)), v);
        }

        // x_u + x_v >= 1 for each edge e=(u,v)
        for (auto e : boost::make_iterator_range(edges(problem.get_graph()))) {
            auto x_u = problem.vertex_to_column(source(e, problem.get_graph()));
            auto x_v = problem.vertex_to_column(target(e, problem.get_graph()));
            lp.add_row(x_u + x_v >= 1);
        }
    }
};

struct vertex_cover_set_solution {
    template <typename Problem, typename GetSolution>
    void operator()(Problem &problem, const GetSolution &solution) {
        // add vertices with column value equal 1 to cover
        for (auto v :
             boost::make_iterator_range(vertices(problem.get_graph()))) {
            if (m_compare.e(solution(problem.vertex_to_column(v)), 1)) {
                problem.add_to_cover(v);
            }
        }
    }

private:
    const paal::utils::compare<double> m_compare;
};

using vertex_cover_ir_components =
    ir::IRcomponents<vertex_cover_init, ir::round_condition_greater_than_half,
                     paal::utils::always_false, vertex_cover_set_solution>;
//! [Iterative Rounding Components Example]

//! [Iterative Rounding Example]
int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, boost::no_property>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

    // sample problem
    std::vector<std::pair<int, int>> edges{ { 0, 1 }, { 0, 2 }, { 1, 2 },
                                            { 1, 3 }, { 1, 4 }, { 1, 5 },
                                            { 5, 0 }, { 3, 4 } };
    std::vector<int> costs{ 1, 2, 1, 2, 1, 5 };

    Graph g(edges.begin(), edges.end(), 6);
    auto vertex_costs = paal::utils::make_array_to_functor(costs);
    std::vector<Vertex> result_cover;
    auto insert_iter = std::back_inserter(result_cover);

    vertex_cover<Graph, decltype(vertex_costs), decltype(insert_iter)> problem(
        g, vertex_costs, insert_iter);

    // solve it
    auto result =
        ir::solve_iterative_rounding(problem, vertex_cover_ir_components());

    // print result
    if (result.first == paal::lp::OPTIMAL) {
        std::cout << "Vertices in the cover:" << std::endl;
        for (auto v : result_cover) {
            std::cout << "Vertex " << v << std::endl;
        }
        std::cout << "Cost of the solution: " << *(result.second) << std::endl;
    } else {
        std::cout << "The instance is infeasible" << std::endl;
    }
    paal::lp::glp::free_env();
    return 0;
}
//! [Iterative Rounding Example]
