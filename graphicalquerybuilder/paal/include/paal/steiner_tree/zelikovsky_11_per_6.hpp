//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file zelikovsky_11_per_6.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-24
 */

#ifndef PAAL_ZELIKOVSKY_11_PER_6_HPP
#define PAAL_ZELIKOVSKY_11_PER_6_HPP

#include <boost/config.hpp>

#include "paal/utils/contract_bgl_adjacency_matrix.hpp"
#include "paal/data_structures/subset_iterator.hpp"
#include "paal/data_structures/metric/metric_to_bgl.hpp"
#include "paal/data_structures/metric/metric_traits.hpp"
#include "paal/data_structures/voronoi/voronoi.hpp"
#include "paal/data_structures/metric/graph_metrics.hpp"
#include "paal/local_search/local_search.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/irange.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

#include <boost/range/combine.hpp>
#include <boost/range/join.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <stack>

namespace paal {

namespace detail {
/**
 * class steiner_tree
 * @brief This is Alexander Zelikovsky 11/6 approximation algorithm for steiner
 tree.
 *
 * Example: <br>
   \snippet zelikovsky_11_per_6_example.cpp Steiner Tree Example
 *
 * example file is steiner_tree_example.cpp
 *
 * @tparam Metric we only use this metric for distances  (Steiner, Terminal) and
 (Terminal, Terminal)
 * @tparam Voronoi models WeakVoronoi (see \ref voronoi). This is a Voronoi
 division where generators are terminals  of the steiner tree.
 */
template <typename Metric, typename Voronoi> class steiner_tree {
    using Idx = int;

  public:
    using MT = data_structures::metric_traits<Metric>;
    using Dist = typename MT::DistanceType;
    using VertexType = typename MT::VertexType;
    static const int SUBSET_SIZE = 3;

    using ThreeTuple = k_tuple_t<Idx, SUBSET_SIZE>;
    using Move = boost::tuple<ThreeTuple, Dist>;
    using ResultSteinerVertices = std::vector<VertexType>;

    /**
     * @brief
     *
     * @param m
     * @param vor
     */
    steiner_tree(const Metric &m, const Voronoi &vor)
        : m_metric(m), m_voronoi(vor),
          N(boost::distance(vor.get_generators())),
          m_save(N) {}

    template <typename OutputIterator>
    void get_result_steiner_vertices(OutputIterator out) {
        ResultSteinerVertices res;

        if (m_voronoi.get_vertices().empty()) {
            return;
        }

        auto ti = irange(N);
        auto subsets = make_three_subset_range(ti.begin(), ti.end());

        auto get_moves = [&](const AMatrix &) {
            return boost::combine(subsets, m_subs_dists | boost::adaptors::transformed(
                                                   utils::remove_reference()));
        };

        auto obj_fun = [&](const AMatrix & m, const Move & t) {
            return gain(t);
        };

        auto commit_move = [&](AMatrix & m, const Move & t) {
            contract(m, get<0>(t));
            res.push_back(m_nearest_vertex[get<0>(t)]);
            return true;
        };

        auto search_components = local_search::make_search_components(
            get_moves, obj_fun, commit_move);

        auto ls_solution = data_structures::metric_to_bgl_with_index(
            m_metric, m_voronoi.get_generators(), m_t_idx);

        fill_sub_dists();

        find_save(ls_solution);
        local_search::local_search(ls_solution,
                                   local_search::best_improving_strategy{},
                                   [ = ](AMatrix & a) {
            find_save(a);
            return true;
        },
                                   utils::always_false(), search_components);

        unique_res(res);
        boost::copy(res, out);
    }

  private:

    // Spanning tree types
    using SpanningTreeEdgeProp = boost::property<boost::edge_index_t, int,
                            boost::property<boost::edge_weight_t, Dist>>;
    using SpanningTree = boost::subgraph<boost::adjacency_list<
        boost::listS, boost::vecS, boost::undirectedS, boost::no_property,
        SpanningTreeEdgeProp>>;
    using GTraits = boost::graph_traits<SpanningTree>;
    using SEdge = typename GTraits::edge_descriptor;

    // Adjacency Matrix types
    using AMatrix = typename data_structures::adjacency_matrix<Metric>::type;
    using MTraits = boost::graph_traits<AMatrix>;
    using MEdge = typename MTraits::edge_descriptor;

    // other types
    using ThreeSubsetsDists = std::vector<Dist>;
    using NearstByThreeSubsets = std::unordered_map<ThreeTuple, VertexType,
        boost::hash<ThreeTuple>>;

    template <typename Iter>
    boost::iterator_range<
        data_structures::subsets_iterator<SUBSET_SIZE, Iter>>
    make_three_subset_range(Iter b, Iter e) {
        return data_structures::make_subsets_iterator_range<SUBSET_SIZE>(b, e);
    }

    void unique_res(ResultSteinerVertices &res) {
        std::sort(res.begin(), res.end());
        auto new_end = std::unique(res.begin(), res.end());
        res.resize(std::distance(res.begin(), new_end));
    }

    void contract(AMatrix &am, const ThreeTuple &t) {
        utils::contract(am, get<0>(t), std::get<1>(t));
        utils::contract(am, get<1>(t), std::get<2>(t));
    }

    Dist gain(const Move &t) {
        auto const &m = m_save;
        Idx a, b, c;
        std::tie(a, b, c) = get<0>(t);

        assert(m(a, b) == m(b, c) || m(b, c) == m(c, a) || m(c, a) == m(a, b));
        return max3(m(a, b), m(b, c), m(c, a)) +
               min3(m(a, b), m(b, c), m(c, a)) - get<1>(t);
    }

    void fill_sub_dists() {
        auto ti = irange(N);

        auto sub_range = make_three_subset_range(ti.begin(), ti.end());
        m_subs_dists.reserve(boost::distance(sub_range));

        // finding nearest vertex to subset
        for (const ThreeTuple &subset : sub_range) {
            // TODO awful coding, need to be changed to loop (using fold
            // form utils/fusion.hpp)
            // TODO There is possible problem, one point could belong to two
            // voronoi regions
            // In our implementation the point will be in exactly one region and
            // there
            // it will not be contained in the range
            auto v_range1 = m_voronoi.get_vertices_for_generator(
                m_t_idx.get_val(std::get<0>(subset)));
            auto v_range2 = m_voronoi.get_vertices_for_generator(
                m_t_idx.get_val(std::get<1>(subset)));
            auto v_range3 = m_voronoi.get_vertices_for_generator(
                m_t_idx.get_val(std::get<2>(subset)));
            auto range = boost::join(boost::join(v_range1, v_range2), v_range3);

            if (boost::empty(range)) {
                m_nearest_vertex[subset] = *m_voronoi.get_vertices().begin();
            } else {
                m_nearest_vertex[subset] = *std::min_element(
                    std::begin(range), std::end(range),
                    utils::make_functor_to_comparator([&](VertexType v) {
                    return dist(v, subset);
                }));
            }
            m_subs_dists.push_back(dist(m_nearest_vertex[subset], subset));
        }
    }

    Dist max3(Dist a, Dist b, Dist c) { return std::max(std::max(a, b), c); }

    Dist min3(Dist a, Dist b, Dist c) { return std::min(std::min(a, b), c); }

    Dist dist(VertexType steiner_point, Idx terminal_idx) {
        return m_metric(steiner_point, m_t_idx.get_val(terminal_idx));
    }

    // minor TODO could by more general somewhere
    Dist dist(VertexType steiner_point, const ThreeTuple &tup) {
        return dist(steiner_point, std::get<0>(tup)) +
               dist(steiner_point, std::get<1>(tup)) +
               dist(steiner_point, std::get<2>(tup));
    }

    /**
     * @brief Constructs spanning tree from current am
     *
     * @return
     */
    SpanningTree get_spanning_tree(const AMatrix &am) {
        // compute spanning tree and write it to  vector
        std::vector<Idx> pm(N);
        boost::prim_minimum_spanning_tree(am, &pm[0]);

        // transform vector into SpanningTree object
        auto const &weight_map = get(boost::edge_weight, am);
        SpanningTree spanning_tree(N);
        for (Idx from = 0; from < N; ++from) {
            if (from != pm[from]) {
                bool succ = add_edge(
                    from, pm[from],
                    SpanningTreeEdgeProp(
                        from, get(weight_map, edge(from, pm[from], am).first)),
                    spanning_tree).second;
                assert(succ);
            }
        }
        return spanning_tree;
    }

    template <typename WeightMap, typename EdgeRange>
    SEdge max_edge(EdgeRange range, const WeightMap &weight_map) const {
        assert(range.first != range.second);
        return *std::max_element(range.first, range.second,
                                 [&](SEdge e, SEdge f) {
            return get(weight_map, e) < get(weight_map, f);
        });
    }

    void create_subgraphs(SpanningTree &g, SpanningTree &g1, SpanningTree &g2) {
        int n = num_vertices(g);
        std::vector<Idx> comps(n);
        boost::connected_components(g, &comps[0]);
        int c1 = comps[0];
        int c2 = -1;

        for (auto i : irange(n)) {
            if (comps[i] == c1) {
                add_vertex(g.local_to_global(i), g1);
            } else {
                assert(c2 == -1 || comps[i] == c2);
                c2 = comps[i];
                add_vertex(g.local_to_global(i), g2);
            }
        }
    }

    // setting m_save(v,w) = max_dist, for each v in g1 and w in g2
    void move_save(const SpanningTree &g1, const SpanningTree &g2,
                   Dist max_dist) {
        auto v1 = vertices(g1);
        auto v2 = vertices(g2);
        for (auto v : boost::make_iterator_range(v1)) {
            for (auto w : boost::make_iterator_range(v2)) {
                auto vg = g1.local_to_global(v);
                auto wg = g2.local_to_global(w);
                m_save(vg, wg) = max_dist;
                m_save(wg, vg) = max_dist;
            }
        }
    }

    // finds the longest edge between each pair of vertices
    // in the spanning tree
    // preforms recursive procedure
    void find_save(const AMatrix &am) {
        auto spanning_tree = get_spanning_tree(am);

        std::stack<SpanningTree *> s;
        s.push(&spanning_tree);

        while (!s.empty()) {
            // TODO delete children at once
            SpanningTree &g = *s.top();
            s.pop();
            int n = num_vertices(g);
            if (n == 1) {
                continue;
            }
            auto e_range = edges(g);
            assert(e_range.first != e_range.second);
            auto const &weight_map = get(boost::edge_weight, g);
            SEdge max_el = max_edge(e_range, weight_map);
            Dist max_dist = get(weight_map, max_el);
            remove_edge(max_el, g);
            SpanningTree &g1 = g.create_subgraph();
            SpanningTree &g2 = g.create_subgraph();
            create_subgraphs(g, g1, g2);

            move_save(g1, g2, max_dist);

            s.push(&g1);
            s.push(&g2);
        }
    }

    const Metric &m_metric;
    const Voronoi &m_voronoi;
    ThreeSubsetsDists m_subs_dists;
    NearstByThreeSubsets m_nearest_vertex;
    int N;
    data_structures::array_metric<Dist> m_save;
    data_structures::bimap<VertexType> m_t_idx;
};
} // !detail

/**
 * @brief 11/6 approximation for steiner_tree problem
 *
 * @tparam Metric
 * @tparam Voronoi
 * @tparam OutputIterator
 * @param m
 * @param v
 * @param out
 */
template <typename Metric, typename Voronoi, typename OutputIterator>
void steiner_tree_zelikovsky11per6approximation(const Metric &m,
                                                const Voronoi &v,
                                                OutputIterator out) {
    detail::steiner_tree<Metric, Voronoi> st(m, v);
    st.get_result_steiner_vertices(out);
}

} // paal

#endif // PAAL_ZELIKOVSKY_11_PER_6_HPP
