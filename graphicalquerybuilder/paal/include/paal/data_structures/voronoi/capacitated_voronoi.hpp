//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file capacitated_voronoi.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-03-20
 */
#ifndef PAAL_CAPACITATED_VORONOI_HPP
#define PAAL_CAPACITATED_VORONOI_HPP

#include "paal/data_structures/metric/metric_traits.hpp"
#include "paal/utils/irange.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/as_array.hpp>
#include <boost/range/numeric.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/graph/successive_shortest_path_nonnegative_weights.hpp>
#include <boost/graph/find_flow_cost.hpp>

#include <unordered_map>

namespace paal {
namespace data_structures {

/**
 * @class capacitated_voronoi
 * @brief This class is assigning vertices demands to capacitated generators in
 * such a way that the total cost is minimized.
 *        The solution is based on the min cost max flow algorithm.
 *
 * @tparam Metric
 * @tparam GeneratorsCapacieties is a functor which for each Generator returns
 * its capacity .
 * @tparam VerticesDemands is a functor which for each vertex returns its
 * demand.
 */
template <typename Metric, typename GeneratorsCapacieties,
          typename VerticesDemands>
class capacitated_voronoi {
  public:
    /**
     * @brief this class store as a distance:
     *          - sum of distances of assigned vertices to its generators
     *          - number of vertices without generator
     *       in the optimum all vertices should be assigned.
     */
    class Dist {
      public:
        typedef typename metric_traits<Metric>::DistanceType DistI;
        Dist() = default;

        /**
         * @brief constructor
         *
         * @param real
         * @param distToFullAssign
         */
        Dist(DistI real, DistI distToFullAssign)
            : m_real_dist(real), m_dist_to_full_assignment(distToFullAssign) {}

        /**
         * @brief operator-
         *
         * @param d
         */
        Dist operator-(Dist d) {
            return Dist(
                m_real_dist - d.m_real_dist,
                m_dist_to_full_assignment - d.m_dist_to_full_assignment);
        }

        /**
         * @brief how many vertices are not covered
         */
        DistI get_dist_to_full_assignment() const {
            return m_dist_to_full_assignment;
        }

        /**
         * @brief sum of distances from vertices to facilities
         */
        DistI get_real_dist() const { return m_real_dist; }

        /**
         * @brief operator==
         *
         * @param d
         */
        bool operator==(Dist d) const {
            return m_real_dist == d.m_real_dist &&
                   m_dist_to_full_assignment == d.m_dist_to_full_assignment;
        }

        /**
         * @brief operator>
         *
         * @param d
         */
        bool operator>(Dist d) const {
            if (m_dist_to_full_assignment > d.m_dist_to_full_assignment) {
                return true;
            } else if (m_dist_to_full_assignment < d.m_dist_to_full_assignment) {
                return false;
            }
            return m_real_dist > d.m_real_dist;
        }

        /**
         * @brief operator+=
         *
         * @param d
         */
        const Dist &operator+=(Dist d) {
            m_real_dist += d.m_real_dist;
            m_dist_to_full_assignment += d.m_dist_to_full_assignment;
            return *this;
        }

        /**
         * @brief operator+
         *
         * @param d
         */
        Dist operator+(Dist d) {
            Dist ret(d);
            ret += *this;
            return ret;
        }

        /**
         * @brief unary -operator
         *
         * @return
         */
        Dist operator-() {
            return Dist(-m_real_dist, -m_dist_to_full_assignment);
        }

        /**
         * @brief Dist  + scalar (interpreted as real distance)
         *
         * @param di
         * @param d
         *
         * @return
         */
        friend Dist operator+(DistI di, Dist d) {
            return Dist(d.m_real_dist + di, d.m_dist_to_full_assignment);
        }

        /**
         * @brief operator<<
         *
         * @tparam Stream
         * @param s
         * @param d
         *
         * @return
         */
        template <typename Stream>
        friend Stream &operator<<(Stream &s, Dist d) {
            return s << d.m_dist_to_full_assignment << " " << d.m_real_dist;
        }

      private:
        DistI m_real_dist;
        DistI m_dist_to_full_assignment;
    };
    typedef typename Dist::DistI DistI;
    typedef typename metric_traits<Metric>::VertexType VertexType;
    typedef std::set<VertexType> Generators;
    typedef std::vector<VertexType> Vertices;

  private:
    typedef boost::adjacency_list<
        boost::listS, boost::vecS, boost::bidirectionalS,
        boost::property<boost::vertex_name_t, VertexType>,
        boost::property<
            boost::edge_capacity_t, DistI,
            boost::property<
                boost::edge_residual_capacity_t, DistI,
                boost::property<boost::edge_reverse_t,
                                boost::adjacency_list_traits<
                                    boost::listS, boost::vecS,
                                    boost::bidirectionalS>::edge_descriptor,
                                boost::property<boost::edge_weight_t, DistI>>>>>
        Graph;
    typedef boost::graph_traits<Graph> GTraits;
    typedef typename GTraits::edge_descriptor ED;
    typedef typename GTraits::edge_iterator EI;
    typedef typename GTraits::in_edge_iterator IEI;
    typedef typename GTraits::vertex_descriptor VD;
    typedef typename boost::property_map<
        Graph, boost::edge_residual_capacity_t>::type ResidualCapacity;
    typedef typename std::unordered_map<VertexType, VD, boost::hash<VertexType>>
        VertexToGraphVertex;

    /**
     * @brief functor transforming edge descriptor into pair :
     * (reindexed source, flow on the edge)
     */
    struct Trans {
        std::pair<VertexType, DistI> operator()(const ED &e) const {
            return std::make_pair(m_v->get_vertex_for_edge(e),
                                  m_v->get_flow_on_edge(e));
        }
        const capacitated_voronoi *m_v;
    };

    typedef boost::transform_iterator<Trans, IEI, std::pair<VertexType, DistI>>
        VForGenerator;

  public:

    /**
     * @brief constructor
     *
     * @param gen
     * @param ver
     * @param m
     * @param gc
     * @param vd
     * @param costOfNoGenerator
     */
    capacitated_voronoi(const Generators &gen, Vertices ver, const Metric &m,
                        const GeneratorsCapacieties &gc,
                        const VerticesDemands &vd,
                        DistI costOfNoGenerator =
                            std::numeric_limits<DistI>::max())
        : m_s(add_vertex_to_graph()), m_t(add_vertex_to_graph()),
          m_vertices(std::move(ver)), m_metric(m), m_generators_cap(gc),
          m_first_generator_id(m_vertices.size() + 2),
          m_cost_of_no_generator(costOfNoGenerator) {
        for (VertexType v : m_vertices) {
            VD vGraph = add_vertex_to_graph(v);
            m_v_to_graph_v.insert(std::make_pair(v, vGraph));
            add_edge_to_graph(m_s, vGraph, 0, vd(v));
        }
        for (VertexType g : gen) {
            add_generator(g);
        }
    }

    /**
     * @brief copy constructor is not default  because of rev graph property
     *
     * @param other
     */
    capacitated_voronoi(const capacitated_voronoi &other)
        : m_dist(other.m_dist), m_dist_prev(other.m_dist_prev),
          m_pred(other.m_pred), m_g(other.m_g), m_s(other.m_s), m_t(other.m_t),
          m_generators(other.m_generators), m_vertices(other.m_vertices),
          m_metric(other.m_metric), m_generators_cap(other.m_generators_cap),
          m_first_generator_id(other.m_first_generator_id),
          m_cost_of_no_generator(other.m_cost_of_no_generator),
          m_v_to_graph_v(other.m_v_to_graph_v),
          m_g_to_graph_v(other.m_g_to_graph_v) {
        auto rev = get(boost::edge_reverse, m_g);
        for (auto e : boost::as_array(edges(m_g))) {
            auto eb = edge(target(e, m_g), source(e, m_g), m_g);
            assert(eb.second);
            rev[e] = eb.first;
        }
    }

    /// returns diff between new cost and old cost
    Dist add_generator(VertexType gen) {
        Dist costStart = get_cost();
        m_generators.insert(gen);
        VD genGraph = add_vertex_to_graph(gen);
        m_g_to_graph_v.insert(std::make_pair(gen, genGraph));
        for (const std::pair<VertexType, VD> &v : m_v_to_graph_v) {
            add_edge_to_graph(v.second, genGraph, m_metric(v.first, gen),
                              std::numeric_limits<DistI>::max());
        }

        add_edge_to_graph(genGraph, m_t, 0, m_generators_cap(gen));

        boost::successive_shortest_path_nonnegative_weights(
            m_g, m_s, m_t, predecessor_map(&m_pred[0]).distance_map(&m_dist[0])
                               .distance_map2(&m_dist_prev[0]));

        return get_cost() - costStart;
    }

    /// returns diff between new cost and old cost
    Dist rem_generator(VertexType gen) {
        Dist costStart = get_cost();
        m_generators.erase(gen);
        auto genGraph = m_g_to_graph_v.at(gen);
        auto rev = get(boost::edge_reverse, m_g);
        auto residual_capacity = get(boost::edge_residual_capacity, m_g);

        // removing flow from the net
        for (const ED &e :
             boost::as_array(in_edges(genGraph, m_g))) {
            bool b;
            VD v = source(e, m_g);
            if (v == m_t) {
                continue;
            }
            DistI cap = residual_capacity[rev[e]];
            ED edgeFromStart;
            std::tie(edgeFromStart, b) = edge(m_s, v, m_g);
            assert(b);
            residual_capacity[edgeFromStart] += cap;
            residual_capacity[rev[edgeFromStart]] -= cap;
        }
        clear_vertex(genGraph, m_g);
        assert(!edge(m_t, genGraph, m_g).second);
        assert(!edge(genGraph, m_t, m_g).second);
        remove_vertex(genGraph, m_g);
        restore_index();

        boost::successive_shortest_path_nonnegative_weights(
            m_g, m_s, m_t, predecessor_map(&m_pred[0]).distance_map(&m_dist[0])
                               .distance_map2(&m_dist_prev[0]));

        return get_cost() - costStart;
    }

    /**
     * @brief getter for generators
     *
     * @return
     */
    const Generators &get_generators() const { return m_generators; }

    /**
     * @brief getter for vertices
     *
     * @return
     */
    const Vertices &get_vertices() const { return m_vertices; }

    /**
     * @brief member function for getting assignment, for generator.
     *
     * @return returns range of pairs; the first element of pair is the Vertex
     * and the second element is the flow from this vertex to given generator
     *
     */
    boost::iterator_range<VForGenerator>
    get_vertices_for_generator(VertexType gen) const {
        IEI ei, end;
        VD v = m_g_to_graph_v.at(gen);
        auto r = in_edges(v, m_g);
        Trans t;
        t.m_v = this;
        return boost::make_iterator_range(VForGenerator(r.first, t),
                                          VForGenerator(r.second, t));
    }

    /**
     * @brief get total cost of the assignment
     *
     * @return
     */
    Dist get_cost() const {
        auto residual_capacity = get(boost::edge_residual_capacity, m_g);
        DistI resCap =
            boost::accumulate(out_edges(m_s, m_g), DistI(0), [&](DistI d, const ED & e) {
            return d + residual_capacity[e];
        });

        DistI cost = boost::find_flow_cost(m_g);
        return Dist(cost, resCap);
    }

    /**
     * @brief operator<<
     *
     * @tparam OStream
     * @param s
     * @param v
     *
     * @return
     */
    template <typename OStream>
    friend OStream &operator<<(OStream &s, capacitated_voronoi &v) {
        s << num_vertices(v.m_g) << ", ";
        s << v.m_s << ", " << v.m_t << "\n";
        auto verticesToDisplay = vertices(v.m_g);
        auto edgesToDisplay = edges(v.m_g);
        auto capacity = get(boost::edge_capacity, v.m_g);
        auto residual_capacity = get(boost::edge_residual_capacity, v.m_g);
        auto name = get(boost::vertex_name, v.m_g);
        for (auto v : boost::as_array(verticesToDisplay)) {
            s << v << "-> " << name[v] << ", ";
        }
        s << "\n";
        for (auto e : boost::as_array(edgesToDisplay)) {
            s << e << "-> " << residual_capacity[e] << "-> " << capacity[e]
              << ", ";
        }
        s << "\n";
        for (int g : v.m_generators) {
            s << g << "\n";
        }
        s << "\n";
        for (int g : v.m_vertices) {
            s << g << "\n";
        }
        s << "\n";
        s << v.m_first_generator_id << "\n";
        s << v.m_cost_of_no_generator << "\n";
        s << "\n";
        for (std::pair<int, int> g : v.m_v_to_graph_v) {
            s << g.first << ", " << g.second << "\n";
        }
        s << "\n";
        for (std::pair<int, int> g : v.m_g_to_graph_v) {
            s << g.first << ", " << g.second << "\n";
        }
        s << "\n";
        return s;
    }

  private:

    /**
     * @brief resores index (name property in the graph)
     */
    void restore_index() {
        const unsigned N = num_vertices(m_g);
        m_g_to_graph_v.clear();
        auto name = get(boost::vertex_name, m_g);
        for (unsigned i : irange(unsigned(m_first_generator_id), N)) {
            m_g_to_graph_v[name[i]] = i;
        }
    }

    /**
     * @brief add vertex to auxiliary graph
     *
     * @param v
     *
     * @return
     */
    VD add_vertex_to_graph(VertexType v = VertexType()) {
        VD vG = add_vertex(boost::property<boost::vertex_name_t, VertexType>(v),
                           m_g);
        int N = num_vertices(m_g);

        m_dist.resize(N);
        m_dist_prev.resize(N);
        m_pred.resize(N);
        return vG;
    }

    /**
     * @brief add edge to auxiliary graph
     *
     * @param v
     * @param w
     * @param weight
     * @param capacity
     */
    void add_edge_to_graph(VD v, VD w, DistI weight, DistI capacity) {
        auto rev = get(boost::edge_reverse, m_g);
        ED e, f;
        e = add_dir_edge(v, w, weight, capacity);
        f = add_dir_edge(w, v, -weight, 0);
        rev[e] = f;
        rev[f] = e;
    }

    /**
     * @brief add directed edge
     *
     * @param v
     * @param w
     * @param weight
     * @param capacity
     *
     * @return
     */
    ED add_dir_edge(VD v, VD w, DistI weight, DistI capacity) {
        bool b;
        ED e;
        auto weightMap = get(boost::edge_weight, m_g);
        auto capacityMap = get(boost::edge_capacity, m_g);
        auto residual_capacity = get(boost::edge_residual_capacity, m_g);
        std::tie(e, b) = add_edge(v, w, m_g);
        assert(b);
        capacityMap[e] = capacity;
        residual_capacity[e] = capacity;
        weightMap[e] = weight;
        return e;
    }

    /**
     * @brief gets flow on edge
     *
     * @param e
     *
     * @return
     */
    DistI get_flow_on_edge(const ED &e) const {
        auto capacityMap = get(boost::edge_capacity, m_g);
        auto residual_capacity = get(boost::edge_residual_capacity, m_g);
        return capacityMap[e] - residual_capacity[e];
    }

    /**
     * @brief get reindexed source for edge
     *
     * @param e
     *
     * @return
     */
    VertexType get_vertex_for_edge(const ED &e) const {
        auto name = get(boost::vertex_name, m_g);
        return name[source(e, m_g)];
    }

    typedef std::vector<DistI> VPropMap;
    VPropMap m_dist;
    VPropMap m_dist_prev;
    std::vector<ED> m_pred;

    Graph m_g;
    VD m_s, m_t;

    Generators m_generators;
    Vertices m_vertices;
    const Metric &m_metric;
    const GeneratorsCapacieties &m_generators_cap;
    const VD m_first_generator_id;
    DistI m_cost_of_no_generator;
    VertexToGraphVertex m_v_to_graph_v;
    VertexToGraphVertex m_g_to_graph_v;
};

} //! data_structures
} //! paal
#endif // PAAL_CAPACITATED_VORONOI_HPP
