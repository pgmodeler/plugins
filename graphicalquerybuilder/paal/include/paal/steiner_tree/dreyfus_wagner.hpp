//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file dreyfus_wagner.hpp
 * @brief Finds optimal Steiner Tree in exponential time.
 * @author Maciej Andrejczuk
 * @version 1.0
 * @date 2013-07-05
 */
#ifndef PAAL_DREYFUS_WAGNER_HPP
#define PAAL_DREYFUS_WAGNER_HPP

#include "paal/data_structures/metric/metric_traits.hpp"
#include "paal/data_structures/metric/graph_metrics.hpp"

#include <unordered_map>
#include <unordered_set>
#include <bitset>

namespace paal {

/**
 * Implements Dreyfus-Wagner algorithm.
 * The algorithm finds optimal Steiner Tree in exponential time, 3^k * n.
 */
template <typename Metric, typename Terminals, typename NonTerminals,
		  unsigned int TerminalsLimit = 32>
class dreyfus_wagner {
  public:
    using MT = data_structures::metric_traits<Metric>;
    using Vertex = typename MT::VertexType;
    using Dist = typename MT::DistanceType;
    using Edge = typename std::pair<Vertex, Vertex>;
    using TerminalsBitSet = typename std::bitset<TerminalsLimit>;
    using State = std::pair<Vertex, TerminalsBitSet>;
    using steiner_elements =  std::unordered_set<Vertex, boost::hash<Vertex>>;

    /**
     * Constructor used for solving Steiner Tree problem.
     */
    dreyfus_wagner(const Metric &cost_map, const Terminals &term,
				   /*const*/ NonTerminals &non_terminals)
        : m_cost_map(cost_map), m_terminals(term),
          m_non_terminals(non_terminals) {

		assert((uint)m_terminals.size() <= TerminalsLimit);
        for (int i = 0; i < (int)m_terminals.size(); i++) {
            m_elements_map[m_terminals[i]] = i;
        }
    }

    /**
     * Finds optimal Steiner Tree.
     * @param start Vertex to start the recurrence from.
     */
    void solve(int start = 0) {
        int n = m_elements_map.size();
        assert(start >= 0 && start < n);
        TerminalsBitSet remaining;
        // set all terminals except 'start' to 1
        for (int i = 0; i < n; i++) {
            remaining.set(i);
        }
        remaining.reset(start);

        m_cost = connect_vertex(m_terminals[start], remaining);
        retrieve_solution_connect(m_terminals[start], remaining);
    }

    /**
     * Gets the optimal Steiner Tree cost.
     */
    Dist get_cost() const { return m_cost; }

    /**
     * Gets edges belonging to optimal tree.
     */
    const std::vector<Edge> &get_edges() const { return m_edges; }

    /**
     * Gets selected Steiner vertices.
     */
    const steiner_elements &get_steiner_elements() const {
        return m_steiner_elements;
    }

    const Metric get_cost_map() const {return m_cost_map;};

//private:
    /*
     * @brief Computes minimal cost of connecting given vertex and a set of
     * other vertices.
     * @param v vertex currently processed
     * @param mask vertices not yet processed has corresponding bits set to 1
     */
    Dist connect_vertex(Vertex v, TerminalsBitSet remaining) {
        if (remaining.none()) {
            return 0;
        }
        if (remaining.count() == 1) {
            int k = smallest_bit(remaining);
            Dist cost = m_cost_map(v, m_terminals[k]);
            m_best_cand[code_state(v, remaining)] =
                std::make_pair(cost, m_terminals[k]);
            return cost;
        }
        // Check in the map if already computed
        auto iter = m_best_cand.find(code_state(v, remaining));
        if (iter != m_best_cand.end()) {
            return iter->second.first;
        }
        Dist best = split_vertex(v, remaining);
        Vertex cand = v;

        auto try_vertex = [&](Vertex w) {
                Dist val = split_vertex(w, remaining);
                val += m_cost_map(v, w);
                if (best < 0 || val < best) {
                    best = val;
                    cand = w;
                }
            };
        for (Vertex w : m_non_terminals) {
            try_vertex(w);
        }
        for (auto w_with_id : m_elements_map) {
            if (!remaining.test(w_with_id.second)) {
                try_vertex(w_with_id.first);
            }
        }
        for (auto vertex_and_terminal_id : m_elements_map) {
            Vertex w = vertex_and_terminal_id.first;
            int terminal_id = vertex_and_terminal_id.second;
            if (!remaining.test(terminal_id)) continue;
            remaining.reset(terminal_id);
            Dist val = connect_vertex(w, remaining);
            val += m_cost_map(v, w);
            remaining.set(terminal_id);

            if (best < 0 || val < best) {
                best = val;
                cand = w;
            }
        }
        m_best_cand[code_state(v, remaining)] = std::make_pair(best, cand);
        return best;
    }

    /**
     * @brief Computes minimal cost by splitting the tree in two parts.
     */
    Dist split_vertex(Vertex v, TerminalsBitSet remaining) {
        if (remaining.count() < 2) {
            return 0;
        }
        // Check in the map if already computed
        auto iter = m_best_split.find(code_state(v, remaining));
        if (iter != m_best_split.end()) {
            return iter->second.first;
        }
        int k = smallest_bit(remaining) +
                1; // optimalization, to avoid checking subset twice
        std::pair<Dist, TerminalsBitSet> best =
            best_split(v, remaining, remaining, k);
        m_best_split[code_state(v, remaining)] = best;
        return best.first;
    }

    /**
     * Generates all splits of given set of vertices and finds the best one.
     */
    std::pair<Dist, TerminalsBitSet> best_split(const Vertex v,
                                                const TerminalsBitSet remaining,
                                                TerminalsBitSet subset, int k) {
        if (k == (int)m_terminals.size()) {
            TerminalsBitSet complement = remaining ^ subset;
            if (!subset.none() && !complement.none()) {
                Dist val =
                    connect_vertex(v, subset) + connect_vertex(v, complement);
                return make_pair(val, subset);
            } else {
                return std::make_pair(-1, NULL);
            }
        } else {
            std::pair<Dist, TerminalsBitSet> ret1, ret2;
            ret1 = best_split(v, remaining, subset, k + 1);
            if (remaining.test(k)) {
                subset.flip(k);
                ret2 = best_split(v, remaining, subset, k + 1);
                if (ret1.first < 0 || ret1.first > ret2.first) {
                    ret1 = ret2;
                }
            }
            return ret1;
        }
    }

    /**
     * Retrieves the path of the optimal solution returned by connect_vertex
     * method.
     */
    void retrieve_solution_connect(Vertex v, TerminalsBitSet remaining) {
        if (remaining.none()) return;
        Vertex next = m_best_cand.at(code_state(v, remaining)).second;

        auto terminal_id_iter = m_elements_map.find(next);
        if (v == next) { // called wagner directly from dreyfus
            retrieve_solution_split(next, remaining);
        } else if (terminal_id_iter == m_elements_map.end() // nonterminal
                || !remaining.test(terminal_id_iter->second)) { // terminal not in remaining
            add_vertex_to_graph(next);
            add_edge_to_graph(v, next);
            retrieve_solution_split(next, remaining);
        } else { // terminal
            add_edge_to_graph(v, next);
            remaining.flip(terminal_id_iter->second);
            retrieve_solution_connect(next, remaining);
        }
    }

    /**
     * Retrieves the path of the optimal solution returned by split_vertex
     * method.
     */
    void retrieve_solution_split(Vertex v, TerminalsBitSet remaining) {
        if (remaining.none()) return;
        TerminalsBitSet split =
            m_best_split.at(code_state(v, remaining)).second;
        retrieve_solution_connect(v, split);
        retrieve_solution_connect(v, remaining ^ split);
    }

    /**
     * Codes current state to the structure that fits as the map key.
     */
    State code_state(Vertex v, TerminalsBitSet remaining) {
        // TODO: can be optimized
        return std::make_pair(v, remaining);
    }

    /**
     * Hash function for State, used in unordered_maps.
     */
    struct state_hash {
        std::size_t operator()(const State &k) const {
            return boost::hash<Vertex>()(k.first) ^
                   (std::hash<TerminalsBitSet>()(k.second) << 1);
        }
    };

    /**
     * Adds the edge to the solution.
     */
    void add_edge_to_graph(Vertex u, Vertex w) {
        Edge e = std::make_pair(u, w);
        m_edges.push_back(e);
    }

    /**
     * Adds Steiner vertex to the solution.
     */
    void add_vertex_to_graph(Vertex v) { m_steiner_elements.insert(v); }

    /**
     * Finds the index of the first nonempty bit in given mask.
     */
    int smallest_bit(TerminalsBitSet mask) {
        int k = 0;
        while (!mask.test(k)) ++k;
        return k;
    }

    const Metric &m_cost_map;            // stores the cost for each edge
    const Terminals &m_terminals;        // terminals to be connected
	/*const*/ NonTerminals &m_non_terminals; // list of all non-terminals

    Dist m_cost;                         // cost of optimal Steiner Tree
    steiner_elements m_steiner_elements; // non-terminals selected for spanning
                                         // tree
    std::vector<Edge> m_edges;           // edges spanning the component

    std::unordered_map<Vertex, int, boost::hash<Vertex>> m_elements_map; // maps Vertex to position
                                                    // in m_terminals vector
    using StateV = std::pair<Dist, Vertex>;
    using StateBM = std::pair<Dist, TerminalsBitSet>;
    std::unordered_map<State, StateV, state_hash> m_best_cand; // stores result
                                                               // of dreyfus
                                                               // method for
                                                               // given state
    std::unordered_map<State, StateBM, state_hash> m_best_split; // stores
                                                                 // result of
                                                                 // wagner
                                                                 // method for
																 // given state
};

/**
 * @brief Creates a dreyfus_wagner object.
 * @tparam TerminalsLimit
 * @tparam Metric
 * @tparam Terminals
 * @tparam NonTerminals
 */
template <unsigned int TerminalsLimit = 32, typename Metric, typename Terminals, typename NonTerminals>
dreyfus_wagner<Metric, Terminals, NonTerminals, TerminalsLimit>
make_dreyfus_wagner(const Metric &metric, const Terminals &terminals,
					/*const*/ NonTerminals &non_terminals) {
    return dreyfus_wagner<Metric, Terminals, NonTerminals, TerminalsLimit>(
        metric, terminals, non_terminals);
}
} // paal

#endif // PAAL_DREYFUS_WAGNER_HPP
