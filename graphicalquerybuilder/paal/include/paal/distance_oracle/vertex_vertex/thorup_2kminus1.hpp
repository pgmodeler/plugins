//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
  * @file thorup_2kminus1.hpp
  * @brief
  * @author Jakub Ocwieja
  * @version 1.0
  * @date 2014-04-28
  */

#ifndef PAAL_THORUP_2KMINUS1_HPP
#define PAAL_THORUP_2KMINUS1_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/irange.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/range/as_array.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>

#include <queue>
#include <unordered_map>
#include <limits>
#include <random>
#include <cmath>

namespace paal {

/**
* @brief 2k-1 approximate distance oracle
*
* @tparam Graph graph
* @tparam EdgeWeightMap edge weight map
* @tparam VertexIndexMap vertex index map
* @tparam Rand random engine
*/
template <  typename Graph,
            typename VertexIndexMap,
            typename EdgeWeightMap,
            typename Rand=std::default_random_engine
            >
class distance_oracle_thorup2kminus1approximation {
    using DT = typename boost::property_traits<EdgeWeightMap>::value_type;
    using VT = typename boost::graph_traits<Graph>::vertex_descriptor;

    //! List of pairs (vertex index, distance to vertex)
    using DVect = std::vector< std::pair<int, DT> >;

    //! Maps vertex in a bunch into a distance to it
    using BunchMap = std::unordered_map<int, DT>;

    //! Index map stored to access internal structures
    VertexIndexMap m_index;

    //! For each vertex v a maximal layer number for which v belongs
    /** A_0 = V
    *   A_{i+1} \subset A_i
    *   A_k = \emptyset
    */
    std::vector< int > m_layer_num;

    //! For each vertex v a list of vertices of consecutive layers closest to v
    std::vector< DVect > m_parent;

    //! For each vertex v a set of vertices w closer to v than any vertex in layer m_layer_num[w]+1
    std::vector< BunchMap > m_bunch;

    /**
    * @brief Fills m_layer_num
    *
    * @param g Graph
    * @param k Approximation parameter = maximal number of layers
    * @param p Probability of being chosen to a next layer
    * @param random_engine Random engine
    *
    * @return Number of nonempty layers
    */
    int choose_layers(const Graph& g, int k, long double p, Rand & random_engine) {
        std::uniform_real_distribution<> dist(0,1);

        int max_layer_num = 0;
        long double logp = log(p);

        for (int ind: irange(num_vertices(g))) {
            m_layer_num[ind] = std::min(k-1, (int)(log(dist(random_engine)) / logp));
            assign_max(max_layer_num, m_layer_num[ind]);
        }

        return max_layer_num+1;
    }

    /**
     * @brief A visitor implementation for compute_parents Dijkstra's algorithm call
     *
     * @tparam NearestMap
     * @tparam Tag
     */
    template <typename NearestMap, typename Tag>
    class nearest_recorder : boost::base_visitor< nearest_recorder<NearestMap, Tag> > {

        //! Stores distances to the closest vertex in a particular layer
        NearestMap m_nearest_map;

    public:
        using event_filter = Tag;

        //! Constructor
        explicit nearest_recorder(NearestMap const nearest_map) : m_nearest_map(nearest_map) {}

        //! Copies nearest value from precdessor
        template <typename Edge>
        void operator()(Edge const e, Graph const &g) const {
            auto nearest = get(m_nearest_map, source(e,g));
            put(m_nearest_map, target(e,g), nearest);
        }

    };

    //! Constructs a visitor for compute_parents Dijkstra's algorithm call
    template <typename NearestMap, typename Tag>
    nearest_recorder<NearestMap, Tag>
    make_nearest_recorder(NearestMap nearest_map, Tag) {
        return nearest_recorder<NearestMap, Tag>{nearest_map};
    }

    /**
    * @brief Fills m_parent for a single layer
    *
    * @param g Graph
    * @param layer_num Number of layer
    */
    void compute_parents(const Graph& g, EdgeWeightMap edge_weight, int layer_num) {
        std::vector<DT> distance(num_vertices(g), std::numeric_limits<DT>::max());
        std::vector<int> nearest(num_vertices(g), -1);
        std::vector<VT> roots;

        for (auto v: boost::as_array(vertices(g))) {
            int v_ind = m_index[v];
            if (m_layer_num[v_ind] >= layer_num) {
                nearest[v_ind] = v_ind;
                distance[v_ind] = DT{};
                roots.push_back(v);
            }
        }

        boost::dijkstra_shortest_paths_no_init(
                g,
                roots.begin(),
                roots.end(),
                boost::dummy_property_map(),
                make_iterator_property_map(distance.begin(), m_index, distance[0]),
                edge_weight,
                m_index,
                utils::less{},
                boost::closed_plus<DT>(),
                DT{},
                boost::make_dijkstra_visitor(make_nearest_recorder(
                        make_iterator_property_map(nearest.begin(), m_index, nearest[0]),
                        boost::on_edge_relaxed{})
                    )
            );

        for (int ind: irange(num_vertices(g))) {
            m_parent[ind].push_back(std::make_pair(nearest[ind], distance[ind]));
        }
    }

     //! A distance type crafted to control logic of compute_cluster Dijkstra's algorithm call
    class cluster_dist {
        //! An actual distance
        DT m_value;
        //! Marks unmodified vertices
        bool m_unmodified;

        //! Private constructor
        cluster_dist(DT value, bool unmodified) :
            m_value(value), m_unmodified(unmodified) {}

    public:
        //! Public constructor
        cluster_dist(DT value = DT{}) :
            m_value(value), m_unmodified(false) {}

        //! Allows to create unmodified distances
        static cluster_dist
        make_limit(DT value = std::numeric_limits<DT>::max()) {
            return cluster_dist(value, true);
        }

        //! A comparator struct adjusted to recognize unmodified values
        /** Unmodified values are not smaller then any modified value. To recognize an umodified value we compare it
         *  with a maximal modified value of DT
         */
        struct less {
            //! A comparison operator
            bool operator()(cluster_dist a, cluster_dist b) const {
                return (a.m_value < b.m_value) && (b.m_unmodified || !a.m_unmodified);
            }
        };

        //! Plus operation struct
        struct plus {
            //! Sum operator
            cluster_dist operator()(cluster_dist a, cluster_dist b) const {
                return cluster_dist(a.m_value + b.m_value, a.m_unmodified || b.m_unmodified);
            }
        };

        //! An accessor to the distance
        const DT value() const {
            return m_value;
        }
    };

    /**
     * @brief A property_map with lazy initialization of distances
     *
     * For each vertex of a graph a distance is initialized with an upper limit on a value which causes edge
     * relaxation in compute_cluster Dijkstra's algorithm.
     */
    class cluster_distance_wrapper :
        public boost::put_get_helper<cluster_dist&, cluster_distance_wrapper>
    {
        //! A wrapped distance table
        std::vector< cluster_dist > *m_distance;

        //! An index map stored to access table structures
        VertexIndexMap m_index;

        //! A pointer to a parent table containing initial values of fields of m_distance
        /** The values stored here are copied into m_distance table when m_distance fields are accessed for the
         *  first time. For each vertex of a graph it contains an upper limit on a value which causes edge relaxation
         *  in compute_cluster Dijkstra's algorithm.
         */
        std::vector< DT > *m_limit;

        //! A table storing last access time to m_distance fields
        std::vector<int> *m_last_accessed;

        //! A value necessary to interpret m_last_accessed
        /** The value is initially different than any value in m_last_accessed.
         *  However, it is not necessarily bigger.
         */
        int m_now;

    public:
        typedef VT key_type;
        typedef cluster_dist value_type;
        typedef value_type& reference;
        typedef boost::lvalue_property_map_tag category;

        /**
         * @brief Constructor
         *
         * @param distance A helper vector - required to have num_vertices(g) fields
         * @param index An index map
         * @param limit Compute_cluster Dijkstra's algorithm relaxation limits
         * @param last_accessed A helper vector - required to have num_vertices(g) fields
         * @param now Required to be different than any last_accessed value
         */
        cluster_distance_wrapper(
                std::vector< cluster_dist > *distance,
                VertexIndexMap index,
                std::vector< DT > *limit,
                std::vector< int > *last_accessed,
                int now) :
            m_distance(distance),
            m_index(index),
            m_limit(limit),
            m_last_accessed(last_accessed),
            m_now(now) {}

        /**
         * @brief Map values accessor
         *
         * @param key Key
         *
         * @return Value
         */
        reference operator[](const key_type& key) const {
            int k_ind = m_index[key];
            if ((*m_last_accessed)[k_ind] != m_now) {
                (*m_last_accessed)[k_ind] = m_now;
                (*m_distance)[k_ind] = cluster_dist::make_limit((*m_limit)[k_ind]);
            }
            return (*m_distance)[k_ind];
        }
    };

    /**
     * @brief A visitor implementation for a compute_cluster Dijkstra's algorithm call
     *
     * @tparam DistanceMap
     * @tparam Tag
     */
    template <typename DistanceMap, typename Tag>
    class cluster_recorder : boost::base_visitor< cluster_recorder<DistanceMap, Tag> > {
        //! Vertex whose cluster is recorded
        int m_w_ind;

        //! A pointer to m_bunch field of the oracle
        std::vector< BunchMap >* m_bunch;

        //! Index map stored to access internal structures
        VertexIndexMap m_index;

        //! A distance map
        DistanceMap m_distance;

    public:
        using event_filter = Tag;

        explicit cluster_recorder(int w_ind, std::vector<BunchMap> *bunch,
                VertexIndexMap index, DistanceMap distance) :
            m_w_ind(w_ind), m_bunch(bunch), m_index(index), m_distance(distance) {}

        template <typename Vertex>
        void operator()(Vertex const v, Graph const &g) const {
            (*m_bunch)[m_index[v]].insert(std::make_pair(m_w_ind, m_distance[v].value()));
        }
    };

    /**
     * @brief
     *
     * @tparam DistanceMap
     * @tparam Tag
     * @param cluster
     * @param index
     * @param distance
     * @param Tag
     *
     * @return A visitor for a compute_cluster Dijkstra's algorithm call
     */
    template <typename DistanceMap, typename Tag>
    cluster_recorder<DistanceMap, Tag>
    make_cluster_recorder(int w_ind, std::vector<BunchMap> *bunch, VertexIndexMap index,
            DistanceMap distance, Tag) {
        return cluster_recorder<DistanceMap, Tag>{w_ind, bunch, index, distance};
    };

    /**
     * @brief Fills bunchs with vertices inside a cluster - a set of vertices
     *        which contains w in its bunch
     *
     * @param g Graph
     * @param edge_weight Edge weights
     * @param w Vertex
     * @param k Number of layers
     * @param limit Dijkstra's algorithm relaxation limits for each layer
     * @param distance A helper vector - required to have num_vertices(g) fields
     * @param last_accessed A helper vector - require to be initialized with negative values
     */
    void compute_cluster(const Graph& g, EdgeWeightMap edge_weight, VT w, int k,
            std::vector< std::vector<DT> > &limit,
            std::vector<cluster_dist> &distance, std::vector<int> &last_accessed) {
        DVect cluster;
        int w_ind = m_index[w];
        int w_layer_num = m_layer_num[w_ind];

        cluster_distance_wrapper distance_wrapper(
                &distance, m_index, &limit[w_layer_num + 1],
                &last_accessed, w_ind);
        distance_wrapper[w] = cluster_dist(DT{});

        boost::dijkstra_shortest_paths_no_color_map_no_init(
                g,
                w,
                boost::dummy_property_map(),
                distance_wrapper,
                edge_weight,
                m_index,
                typename cluster_dist::less(),
                typename cluster_dist::plus(),
                cluster_dist(std::numeric_limits<DT>::max()),
                cluster_dist(DT{}),
                boost::make_dijkstra_visitor(make_cluster_recorder(
                        w_ind, &m_bunch, m_index, distance_wrapper,
                        boost::on_examine_vertex{})
                    )
            );
    }

    /**
    * @brief Fills m_bunch
    *
    * @param g Graph
    * @param edge_weight Edge weight
    * @param k Number of layers
    */
    void compute_bunchs(const Graph& g, EdgeWeightMap edge_weight, int k) {
        //! Initialization of reusable structures
        std::vector< std::vector<DT> > limit(k+1,
                std::vector<DT>(num_vertices(g), std::numeric_limits<DT>::max()));
        for (int l: irange(k)) {
            for (int i: irange(num_vertices(g))) {
                limit[l][i] = m_parent[i][l].second;
            }
        }
        std::vector<cluster_dist> distance(num_vertices(g));
        std::vector<int> last_accessed(num_vertices(g), -1);

        for (auto v: boost::as_array(vertices(g))) {
            compute_cluster(g, edge_weight, v, k, limit, distance, last_accessed);
        }
    }

public:

    /**
    * @brief Constructor
    *
    * @param g graph
    * @param index vertex index map
    * @param edge_weight edge weight map
    * @param k approximation parameter
    * @param random_engine random engine
    */
    distance_oracle_thorup2kminus1approximation(const Graph &g,
            VertexIndexMap index,
            EdgeWeightMap edge_weight,
            int k,
            Rand && random_engine = Rand(5426u)) :
        m_index(index),
        m_layer_num(num_vertices(g)),
        m_parent(num_vertices(g)),
        m_bunch(num_vertices(g))
            {
        long double p = powl(num_vertices(g), -1./k);
        k = choose_layers(g, k, p, random_engine);
        for (int layer_num: irange(k)) {
            compute_parents(g, edge_weight, layer_num);
        }
        compute_bunchs(g, edge_weight, k);
    }

    //! Returns an 2k-1 approximate distance between two vertices in O(k) time
    /** Returns a distance of path going through one of parents of u or v */
    DT operator()(VT u, VT v) const {
        int u_ind = m_index[u], v_ind = m_index[v];
        typename std::unordered_map<int, DT>::const_iterator it;
        int l = 0;
        std::pair<int, DT> middle_vertex = m_parent[u_ind][l];
        while ((it = m_bunch[v_ind].find(middle_vertex.first)) == m_bunch[v_ind].end()) {
            ++l;
            middle_vertex = m_parent[v_ind][l];
            std::swap(u_ind, v_ind);
        }
        //! Returns d(v, middle) + d(middle, u)
        return it->second + middle_vertex.second;
    }
};

/**
* @brief
*
* @tparam Graph
* @tparam EdgeWeightMap
* @tparam VertexIndexMap
* @tparam Rand
* @param g - given graph
* @param k - approximation parameter
* @param index - graph index map
* @param edge_weight - graph edge weight map
* @param random_engine - random engine
*
* @return 2k-1 approximate distance oracle
*/
template <  typename Graph,
            typename EdgeWeightMap,
            typename VertexIndexMap,
            typename Rand=std::default_random_engine
         >
distance_oracle_thorup2kminus1approximation<Graph, VertexIndexMap, EdgeWeightMap, Rand>
make_distance_oracle_thorup2kminus1approximation(
        const Graph &g,
        const int k,
        VertexIndexMap index,
        EdgeWeightMap edge_weight,
        Rand && random_engine = Rand(5426u)) {
    return distance_oracle_thorup2kminus1approximation<Graph,
    VertexIndexMap,
    EdgeWeightMap,
    Rand>(g, index, edge_weight, k, std::move(random_engine));
}

/**
* @brief
*
* @tparam Graph
* @tparam P
* @tparam T
* @tparam R
* @tparam Rand
* @param g - given graph
* @param k - approximation parameter
* @param params - named parameters
* @param random_engine - random engine
*
* @return 2k-1 approximate distance oracle
*/
template <  typename Graph,
            typename P = char,
            typename T = boost::detail::unused_tag_type,
            typename R = boost::no_property,
            typename Rand=std::default_random_engine
         >
auto
make_distance_oracle_thorup2kminus1approximation(
        const Graph &g,
        const int k,
        const boost::bgl_named_params<P, T, R>& params = boost::no_named_parameters(),
        Rand && random_engine = Rand(5426u))
    -> distance_oracle_thorup2kminus1approximation<Graph,
    decltype(choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index)),
    decltype(choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight)),
    Rand> {
    return make_distance_oracle_thorup2kminus1approximation(g,
            k,
            choose_const_pmap(get_param(params, boost::vertex_index), g, boost::vertex_index),
            choose_const_pmap(get_param(params, boost::edge_weight), g, boost::edge_weight),
            std::move(random_engine));
}

} //paal

#endif // PAAL_THORUP_2KMINUS1_HPP
