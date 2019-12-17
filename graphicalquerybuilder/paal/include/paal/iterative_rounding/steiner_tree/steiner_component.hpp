//=======================================================================
// Copyright (c) 2013 Maciej Andrejczuk
//               2014 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file steiner_component.hpp
 * @brief
 * @author Maciej Andrejczuk, Piotr Wygocki
 * @version 1.0
 * @date 2013-08-01
 */
#ifndef PAAL_STEINER_COMPONENT_HPP
#define PAAL_STEINER_COMPONENT_HPP

#include "paal/data_structures/metric/basic_metrics.hpp"
#include "paal/steiner_tree/dreyfus_wagner.hpp"
#include "paal/utils/irange.hpp"

#include <boost/range/algorithm/transform.hpp>
#include <boost/range/join.hpp>

#include <iosfwd>
#include <set>

namespace paal {
namespace ir {

/**
 * @class steiner_component
 * @brief Class represents k-components of Steiner Tree.
 * Component is a subtree whose terminals coincide with leaves.
 */
template <typename Vertex, typename Dist>
class steiner_component {
public:
    using Edge = typename std::pair<Vertex, Vertex>;
    using Vertices = typename std::vector<Vertex>;

    ///constructor
    template<typename Metric, typename Terminals>
    steiner_component(const Metric & cost_map, Vertices terminals, const Terminals& steiner_vertices) :
        m_terminals(std::move(terminals)), m_size(m_terminals.size()) {

        auto all_elements = boost::join(m_terminals, steiner_vertices);
        data_structures::array_metric<typename data_structures::metric_traits<Metric>::DistanceType>
            fast_metric(cost_map, all_elements);
        auto term_nr = boost::distance(m_terminals);
        auto all_elements_nr = boost::distance(all_elements);
        auto dw = paal::make_dreyfus_wagner(fast_metric,
                    irange(term_nr),
                    irange(int(term_nr), int(all_elements_nr)));
        dw.solve();
        m_cost = dw.get_cost();
        auto &steiner = dw.get_steiner_elements();
        m_steiner_elements.resize(steiner.size());
        auto id_to_elem = [&](int i){
                if(i < term_nr) {
                    return m_terminals[i];
                } else {
                    return steiner_vertices[i - term_nr];
                }
        };
        boost::transform(steiner, m_steiner_elements.begin(), id_to_elem);
        m_edges.resize(dw.get_edges().size());
        boost::transform(dw.get_edges(), m_edges.begin(), [=](std::pair<int, int> e) {
            return std::make_pair(id_to_elem(e.first), id_to_elem(e.second));
        });
    }

    /**
     * @brief Each component has versions, where sink is chosen from its
     * terminals
     */
    Vertex get_sink(int version) const {
        assert(version < count_terminals());
        return m_terminals[version];
    }

    /**
     * Returns vector composed of component's terminals.
     */
    const Vertices &get_terminals() const { return m_terminals; }

    /**
     * Returns vector composed of component's nonterminals, i.e. Steiner
     * elements.
     */
    const Vertices &get_steiner_elements() const {
        return m_steiner_elements;
    }

    /**
     * Returns edges spanning the component.
     */
    const std::vector<Edge> &get_edges() const { return m_edges; }

    /**
     * Returns degree of component, i.e. number of terminals.
     */
    int count_terminals() const { return m_size; }

    /**
     * Returns minimal cost of spanning a component.
     */
    Dist get_cost() const { return m_cost; }

    /**
     * Prints the component.
     */
    friend std::ostream &operator<<(std::ostream &stream,
                                    const steiner_component &component) {
        for (int i = 0; i < component.m_size; i++) {
            stream << component.m_terminals[i] << " ";
        }
        stream << ": ";
        for (auto edge : component.m_edges) {
            stream << "(" << edge.first << "," << edge.second << ") ";
        }
        stream << component.m_cost;
        return stream;
    }

  private:
    const Vertices m_terminals; // terminals of the component
    int m_size;                           // m_terminals.size()
    Dist m_cost; // minimal cost of spanning the component
    Vertices m_steiner_elements; // non-terminals selected for
                                            // spanning tree
    std::vector<Edge> m_edges;              // edges spanning the component
};

} // ir
} // paal

#endif // PAAL_STEINER_COMPONENT_HPP
