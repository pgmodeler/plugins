//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_strategy.hpp
 * @brief
 * @author Maciej Andrejczuk
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef PAAL_STEINER_STRATEGY_HPP
#define PAAL_STEINER_STRATEGY_HPP

#include "paal/data_structures/bimap.hpp"
#include "paal/data_structures/subset_iterator.hpp"
#include "paal/iterative_rounding/steiner_tree/steiner_components.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/range/as_array.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/random/discrete_distribution.hpp>

#include <random>
#include <unordered_set>
#include <vector>


namespace paal {
namespace ir {

/**
 * Generates all the components possible.
 * It iterates over all subsets of terminals with no more than K elements.
 */
class steiner_tree_all_generator {
private:
    template<typename Metric>
    using MTraits = typename data_structures::metric_traits<Metric>;

public:
    /// Constructor.
    steiner_tree_all_generator(int K = 4) : m_component_max_size(K) {}

    /// Generates all possible components.
    template<typename Metric, typename Terminals>
    void gen_components(const Metric& cost_map, const Terminals& terminals,
            const Terminals& steiner_vertices,
            steiner_components<
                typename MTraits<Metric>::VertexType,
                typename MTraits<Metric>::DistanceType>& components) {

        using Vertex = typename MTraits<Metric>::VertexType;
        using Dist = typename MTraits<Metric>::DistanceType;
        std::vector<Vertex> current_terminals;
        gen_all_components<Vertex, Dist>(components, 0, terminals.size(),
            current_terminals, cost_map, terminals, steiner_vertices);
    }
private:
    template<typename Vertex, typename Dist, typename Metric, typename Terminals>
    void gen_all_components(steiner_components<Vertex, Dist>& components,
            int first_avail, int last, std::vector<Vertex>& curr,
            const Metric& cost_map, const Terminals& terminals,
            const Terminals& steiner_vertices) {

        if (curr.size() > 1) {
            steiner_component<Vertex, Dist> c(cost_map, curr, steiner_vertices);
            components.add(std::move(c));
        }
        if ((int) curr.size() >= m_component_max_size)
            return;
        for (int i = first_avail; i < last; ++i) {
            curr.push_back(terminals[i]);
            gen_all_components(components, i + 1, last, curr, cost_map,
                terminals, steiner_vertices);
            curr.pop_back();
        }
    }

    int m_component_max_size;
};

namespace detail {

template<typename Vertex>
struct vertex_filter {
    bool operator()(Vertex v) const { return vertices.find(v) == vertices.end(); }
    std::unordered_set<Vertex> vertices;
};

}// detail

/**
 * Generates all the components possible based on the underlying graph.
 * It iterates over all subsets of terminals with no more than K elements.
 */
template<typename Graph, typename Vertex, typename Terminals>
class steiner_tree_graph_all_generator {
private:
    template<typename Metric>
    using MTraits = typename data_structures::metric_traits<Metric>;

public:
    /// Constructor.
    steiner_tree_graph_all_generator(const Graph& graph,
        const Terminals& terminals, int K = 4) : m_component_max_size(K),
            m_index(terminals),
            m_terminals_graph(m_index.size()) {
        initialize_terminals_graph(graph, terminals);
    }

    /// Generates all possible components.
    template<typename Metric>
    void gen_components(const Metric& cost_map, const Terminals& terminals,
            const Terminals& steiner_vertices,
            steiner_components<
                typename MTraits<Metric>::VertexType,
                typename MTraits<Metric>::DistanceType>& components) {

        using Dist = typename MTraits<Metric>::DistanceType;
        merge_vertices<Dist>(cost_map);
        std::vector<Vertex> current_terminals;
        gen_all_components<Dist>(components, 0, terminals.size(),
            current_terminals, cost_map, terminals, steiner_vertices);
    }

private:
    using AuxGraph = boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS>;
    using VertexIndex = data_structures::bimap<Vertex>;

    int m_component_max_size;
    VertexIndex m_index;
    AuxGraph m_terminals_graph;

    void initialize_terminals_graph(const Graph& graph, const Terminals& terminals) {
        detail::vertex_filter<Vertex> filter;
        filter.vertices.insert(terminals.begin(), terminals.end());
        auto index = get(boost::vertex_index, graph);
        std::vector<int> components(num_vertices(graph));
        for (auto u : terminals) {
            for (auto v : terminals) {
                if (u == v) continue;
                filter.vertices.erase(u);
                filter.vertices.erase(v);
                boost::filtered_graph<Graph, boost::keep_all, decltype(filter)>
                    fg(graph, boost::keep_all{}, filter);
                boost::connected_components(fg, &components[0]);
                if (components[index[u]] == components[index[v]]) {
                    add_edge(get_idx(u), get_idx(v), m_terminals_graph);
                }
                filter.vertices.insert(u);
                filter.vertices.insert(v);
            }
        }
    }

    template<typename Dist, typename Metric>
    void merge_vertices(const Metric& cost_map) {
        auto range = vertices(m_terminals_graph);
        for (auto v_pair : data_structures::make_subsets_iterator_range<2>(
                range.first, range.second)) {
            auto i = std::get<0>(v_pair);
            auto j = std::get<1>(v_pair);
            if (cost_map(get_val(i), get_val(j)) == Dist{}) {
                merge_vertices(i, j);
                merge_vertices(j, i);
            }
        }
    }

    void merge_vertices(Vertex trg, Vertex src) {
        for (auto v : boost::as_array(adjacent_vertices(src,
                m_terminals_graph))) {
            if (trg != static_cast<Vertex>(v)) {
                add_edge(trg, v, m_terminals_graph);
            }
        }
    }

    auto get_idx(Vertex v) const -> decltype(m_index.get_idx(v)) {
        return m_index.get_idx(v);
    }

    auto get_val(int idx) const -> decltype(m_index.get_val(idx)) {
        return m_index.get_val(idx);
    }

    bool is_graph_component(const std::vector<Vertex>& comp) const {
        for (auto term_pair : data_structures::make_subsets_iterator_range<2>(
                comp.begin(), comp.end())) {
            if (!edge(
                    get_idx(std::get<0>(term_pair)),
                    get_idx(std::get<1>(term_pair)),
                    m_terminals_graph).second) {
                return false;
            }
        }
        return true;
    }

    template<typename Dist, typename Metric>
    void gen_all_components(steiner_components<Vertex, Dist>& components,
            int first_avail, int last, std::vector<Vertex>& curr,
            const Metric& cost_map, const Terminals& terminals,
            const Terminals& steiner_vertices) {

        // TODO implement a subset_iterator with K passed as a parameter
        // not as a template parameter (also in the gen_all_components method
        // in steiner_tree_all_generator)
        if (curr.size() > 1) {
            if (!is_graph_component(curr))
                return;
            steiner_component<Vertex, Dist> c(cost_map, curr, steiner_vertices);
            components.add(std::move(c));
        }
        if ((int) curr.size() >= m_component_max_size)
            return;
        for (int i = first_avail; i < last; ++i) {
            curr.push_back(terminals[i]);
            gen_all_components(components, i + 1, last, curr, cost_map,
                terminals, steiner_vertices);
            curr.pop_back();
        }
    }
};

/**
 * Makes a graph_all_generator object.
 */
template<typename Vertex, typename Graph, typename Terminals>
steiner_tree_graph_all_generator<Graph, Vertex, Terminals>
make_steiner_tree_graph_all_generator(const Graph& graph,
        const Terminals& terminals, int K = 4) {
    return steiner_tree_graph_all_generator<Graph, Vertex, Terminals>(
        graph, terminals, K);
}

/**
 * Generates specified number of components by selecting random elements.
 */
class steiner_tree_random_generator {
public:
    /// Constructor.
    steiner_tree_random_generator(int N = 100, int K = 3) :
            m_iterations(N), m_component_max_size(K) {
    }

    /// Generates a specified number of components by selecting random elements.
    template<typename Metric, typename Terminals>
    void gen_components(const Metric& cost_map, const Terminals & terminals,
            const Terminals& steiner_vertices,
            steiner_components<
                typename data_structures::metric_traits<Metric>::VertexType,
                typename data_structures::metric_traits<Metric>::DistanceType>& components) {

        using Vertex = typename data_structures::metric_traits<Metric>::VertexType;
        using Dist = typename data_structures::metric_traits<Metric>::DistanceType;
        if (terminals.size() < 2) {
            return;
        }
        for (int i = 0; i < m_iterations; ++i) {
            std::set<Vertex> curr;
            while ((int)curr.size() < m_component_max_size) {
                if (curr.size() > 1) {
                    int c =
                        (int)rand() %
                        m_component_max_size; // TODO: Is this fair probability?
                    if (c == 0) {
                        break;
                    }
                }
                int r = (int)rand() % terminals.size();
                curr.insert(terminals[r]);
            }
            std::vector<Vertex> elements(curr.begin(), curr.end());
            steiner_component<Vertex, Dist> c(cost_map, elements, steiner_vertices);
            components.add(std::move(c));
        }
        // TODO some terminals may not be in any component
    }

  private:
    int m_iterations;
    int m_component_max_size;
};

/**
 * Generates specified number of components by randomly selecting elements
 * with probability dependent on distance from vertices already selected.
 */
class steiner_tree_smart_generator {
    std::default_random_engine m_rng;
public:
    /// Constructor.
    steiner_tree_smart_generator(int N = 100, int K = 3, std::default_random_engine rng = std::default_random_engine{}) :
            m_iterations(N), m_component_max_size(K) {
    }

    /// Generates components.
    template<typename Metric, typename Terminals>
    void gen_components(const Metric& cost_map, const Terminals& terminals,
            const Terminals& steiner_vertices,
            steiner_components<
                    typename data_structures::metric_traits<Metric>::VertexType,
                    typename data_structures::metric_traits<Metric>::DistanceType>& components) {

        using Vertex = typename data_structures::metric_traits<Metric>::VertexType;
        using Dist = typename data_structures::metric_traits<Metric>::DistanceType;
        std::vector<Vertex> elements;
        std::vector<double> prob;
        for (Vertex start : terminals) {
            for (int i = 0; i < m_iterations; ++i) {
                elements.clear();
                elements.push_back(start);
                int limit = 2 + rand() % (m_component_max_size - 1);
                while ((int)elements.size() < limit) {
                    prob.resize(terminals.size());
                    for (int k = 0; k < (int)prob.size(); ++k) {
                        for (auto e : elements) {
                            if (e == terminals[k]) {
                                prob[k] = 0;
                                break;
                            }
                            int cost = cost_map(e, terminals[k]);
                            assert(cost > 0);
                            assign_max(prob[k], 1. / cost);
                        }
                    }
                    auto selected = boost::random::discrete_distribution<std::size_t>(prob)(m_rng);
                    if (selected == prob.size()) break;
                    elements.push_back(terminals[selected]);
                }
                boost::erase(elements, boost::unique<boost::return_found_end>(boost::sort(elements)));

                steiner_component<Vertex, Dist> c(cost_map, elements, steiner_vertices);
                components.add(std::move(c));
            }
        }
    }

  private:
    int m_iterations;
    int m_component_max_size;
};

} // ir
} // paal

#endif // PAAL_STEINER_STRATEGY_HPP
