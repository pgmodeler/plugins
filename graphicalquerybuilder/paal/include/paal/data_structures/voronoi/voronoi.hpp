//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file voronoi.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
#ifndef PAAL_VORONOI_HPP
#define PAAL_VORONOI_HPP

#include "voronoi_traits.hpp"

#include "paal/data_structures/metric/metric_traits.hpp"
#include "paal/utils/functors.hpp"

#include <boost/range/as_array.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_set>
#include <unordered_map>
#include <map>
#include <cassert>
#include <climits>

namespace paal {
namespace data_structures {

/**
 * @class voronoi
 * @brief simple implementation of the \ref voronoi concept.
 *
 * @tparam Metric
 */
template <typename Metric> class voronoi {
  public:
    typedef typename metric_traits<Metric>::VertexType VertexType;
    typedef std::multimap<VertexType, VertexType> GeneratorsToVertices;
    typedef std::unordered_set<VertexType, boost::hash<VertexType>>
        GeneratorsSet;
    typedef typename metric_traits<Metric>::DistanceType Dist;
    // TODO change to vector
    typedef GeneratorsSet Vertices;
  private:
    typedef std::unordered_map<VertexType,
                               typename GeneratorsToVertices::iterator,
                               boost::hash<VertexType>> VerticesToGenerators;

    VerticesToGenerators m_vertices_to_generators;
    GeneratorsToVertices m_generators_to_vertices;
    Vertices m_vertices;
    GeneratorsSet m_generators;

    const Metric &m_metric;
    const Dist m_cost_of_no_generator;
public:

    /**
     * @brief Constructor
     *
     * @param generators
     * @param vertices
     * @param m
     * @param costOfNoGenerator
     */
    voronoi(const GeneratorsSet &generators, Vertices vertices, const Metric &m,
            Dist costOfNoGenerator = std::numeric_limits<Dist>::max())
        : m_vertices(std::move(vertices)), m_metric(m),
          m_cost_of_no_generator(costOfNoGenerator) {
        for (VertexType f : generators) {
            add_generator(f);
        }
    }

    /**
     * @brief Copy constructor
     *
     * @param v
     */
    voronoi(const voronoi &v)
        : m_generators_to_vertices(v.m_generators_to_vertices),
          m_vertices(v.m_vertices), m_generators(v.m_generators),
          m_metric(v.m_metric),
          m_cost_of_no_generator(v.m_cost_of_no_generator) {
        auto b = m_generators_to_vertices.begin();
        auto e = m_generators_to_vertices.end();
        for (; b != e; ++b) {
            m_vertices_to_generators.insert(std::make_pair(b->second, b));
        }
    }

    /**
     * @brief Move constructor
     *
     * @param v
     */
    voronoi(voronoi &&v)
        : m_vertices_to_generators(std::move(v.m_vertices_to_generators)),
          m_generators_to_vertices(std::move(v.m_generators_to_vertices)),
          m_vertices(std::move(v.m_vertices)),
          m_generators(std::move(v.m_generators)), m_metric(v.m_metric),
          m_cost_of_no_generator(v.m_cost_of_no_generator) {}

    /// returns diff between new cost and old cost
    Dist add_generator(VertexType f) {
        Dist cost = Dist();
        m_generators.insert(f);

        // first generatorsility
        if (m_generators.size() == 1) {
            m_vertices_to_generators.clear();
            m_generators_to_vertices.clear();
            for (VertexType v : m_vertices) {
                m_vertices_to_generators[v] =
                    m_generators_to_vertices.insert(std::make_pair(f, v));
                cost += m_metric(v, f);
            }

            cost = cost - m_cost_of_no_generator;

        } else {
            for (VertexType v : m_vertices) {
                Dist d = m_metric(v, f) - dist(v);
                if (d < 0) {
                    cost += d;
                    assign(v, f);
                }
            }
        }
        return cost;
    }

    /// returns diff between new cost and old cost
    Dist rem_generator(VertexType f) {
        Dist cost = Dist();
        if (m_generators.size() == 1) {
            cost = m_cost_of_no_generator;
            for (VertexType v : m_vertices) {
                cost -= dist(v);
            }
            m_vertices_to_generators.clear();
            m_generators_to_vertices.clear();
        } else {
            auto op =
                std::bind(utils::not_equal_to(), f, std::placeholders::_1);
            auto begin = m_generators_to_vertices.lower_bound(f);
            auto end = m_generators_to_vertices.upper_bound(f);
            for (; begin != end;) {
                auto v = begin->second;
                // using the fact that generators is a map
                //(with other containers you have to be careful cause of iter
                // invalidation)
                ++begin;
                cost -= dist(v);
                cost += adjust_vertex(v, op);
            }
        }
        m_generators.erase(f);
        return cost;
    }

    /**
     * @brief getter for generators
     *
     * @return
     */
    const GeneratorsSet &get_generators() const { return m_generators; }

    /**
     * @brief getter for vertices
     *
     * @return
     */
    const Vertices &get_vertices() const { return m_vertices; }

    /**
     * @brief getter for vertices assigned to specific generator
     *
     * @param g
     */
    auto get_vertices_for_generator(VertexType g) const ->
    decltype(boost::as_array(m_generators_to_vertices.equal_range(g) |
             boost::adaptors::map_values))
    {
        return boost::as_array(m_generators_to_vertices.equal_range(g) |
               boost::adaptors::map_values);
    }

    ///operator==
    bool operator==(const voronoi & vor) const {
        return boost::equal(m_generators_to_vertices, vor.m_generators_to_vertices) &&
               m_cost_of_no_generator == vor.m_cost_of_no_generator &&
               m_metric == vor.m_metric;
    }

  private:

    /**
     * @brief distance of vertex to closest generator
     *
     * @param v
     *
     * @return
     */
    Dist dist(VertexType v) { return m_metric(v, vertex_to_generators(v)); }

    /**
     * @brief find new generator for vertex
     *             only generators satisfying filer condition are considered
     *
     * @tparam Filter
     * @param v
     * @param filter
     *
     * @return
     */
    template <typename Filter = utils::always_true>
    Dist adjust_vertex(VertexType v, Filter filter = Filter()) {
        bool init = true;
        Dist d = Dist();
        VertexType f_best = VertexType();
        for (VertexType f : m_generators) {
            if (filter(f)) {
                Dist td = m_metric(v, f);
                if (init || td < d) {
                    f_best = f;
                    d = td;
                    init = false;
                }
            }
        }
        assert(!init);
        assign(v, f_best);
        return d;
    }

    /**
     * @brief get generator for given vertex
     *
     * @param v
     *
     * @return
     */
    VertexType vertex_to_generators(VertexType v) const {
        auto i = m_vertices_to_generators.find(v);
        assert(i != m_vertices_to_generators.end());
        return i->second->first;
    }

    /**
     * @brief assign vertex to generator
     *
     * @param v
     * @param f
     */
    void assign(VertexType v, VertexType f) {
        auto prev = m_vertices_to_generators.at(v);
        m_generators_to_vertices.erase(prev);
        m_vertices_to_generators[v] =
            m_generators_to_vertices.insert(std::make_pair(f, v));
    }

};


namespace detail {
    template <typename Metric>
    using v_t = typename metric_traits<Metric>::VertexType;

    template <typename Metric>
    using generators_set_t = std::unordered_set<v_t<Metric>, boost::hash<v_t<Metric>>>;

    template <typename Metric>
    using dist_t = typename metric_traits<Metric>::DistanceType;
}

///make for voronoi
template <typename Metric>
voronoi<Metric> make_voronoi(
        const detail::generators_set_t<Metric> & generators,
        detail::generators_set_t<Metric> vertices,
        const Metric & metric,
        detail::dist_t<Metric> costOfNoGenerator = std::numeric_limits<detail::dist_t<Metric>>::max())
{
    return voronoi<Metric>(generators, std::move(vertices), metric, costOfNoGenerator);
}

/**
 * @brief specialization of voronoi_traits
 *
 * @tparam Metric
 */
template <typename Metric>
struct voronoi_traits<voronoi<Metric>> : public _voronoi_traits<
    voronoi<Metric>, typename metric_traits<Metric>::VertexType> {};
};
};

#endif // PAAL_VORONOI_HPP
