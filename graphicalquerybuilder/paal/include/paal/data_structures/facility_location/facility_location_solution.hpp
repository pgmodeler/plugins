//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file facility_location_solution.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_FACILITY_LOCATION_SOLUTION_HPP
#define PAAL_FACILITY_LOCATION_SOLUTION_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE

#include "facility_location_solution_traits.hpp"

#include "paal/data_structures/voronoi/voronoi.hpp"

#include <unordered_set>
#include <cassert>
#include <type_traits>

namespace paal {
namespace data_structures {

/**
 * @brief describes solution to facility location
 * The initial solution is passed as voronoi, which has to be the model of the
 * \ref voronoi concept.
 * The generators of the voronoi are the facilities and the vertices are the
 * clients.
 *
 * @tparam FacilityCost
 * @tparam VoronoiType
 */
template <typename FacilityCost, typename VoronoiType>
class facility_location_solution {
public:
    typedef voronoi_traits<VoronoiType> VT;
    typedef typename VT::VertexType VertexType;
    typedef typename VT::DistanceType Dist;
    typedef typename VT::GeneratorsSet ChosenFacilitiesSet;
    typedef std::unordered_set<VertexType, boost::hash<VertexType>>
        UnchosenFacilitiesSet;

private:

    VoronoiType m_voronoi;
    UnchosenFacilitiesSet m_unchosen_facilities;
    const FacilityCost &m_fac_costs;
public:

    /**
     * @brief constructor
     *
     * @param voronoi
     * @param uf
     * @param c
     */
    facility_location_solution(VoronoiType voronoi, UnchosenFacilitiesSet uf,
                               const FacilityCost &c)
        : m_voronoi(std::move(voronoi)), m_unchosen_facilities(std::move(uf)),
          m_fac_costs(c) {}

    /// returns diff between new cost and old cost
    Dist add_facility(VertexType f) {
        assert(m_unchosen_facilities.find(f) != m_unchosen_facilities.end());
        m_unchosen_facilities.erase(f);

        return m_fac_costs(f) + m_voronoi.add_generator(f);
    }

    /// returns diff between new cost and old cost
    Dist rem_facility(VertexType f) {
        assert(m_unchosen_facilities.find(f) == m_unchosen_facilities.end());
        m_unchosen_facilities.insert(f);

        return -m_fac_costs(f) + m_voronoi.rem_generator(f);
    }

    /// getter for unchosen facilities
    const UnchosenFacilitiesSet &get_unchosen_facilities() const {
        return m_unchosen_facilities;
    }

    /// setter for unchosen facilities
    const ChosenFacilitiesSet &get_chosen_facilities() const {
        return m_voronoi.get_generators();
    }

    /// gets clients assigned to specific facility
    auto get_clients_for_facility(VertexType f) const ->
    decltype(m_voronoi.get_vertices_for_generator(f))
    {
        return m_voronoi.get_vertices_for_generator(f);
    }

    /// gets voronoi
    const VoronoiType &get_voronoi() const { return m_voronoi; }

};

/**
 * @brief traits for facility_location_solution
 *
 * @tparam FacilityCost
 * @tparam voronoi
 */
template <typename FacilityCost, typename Voronoi>
class facility_location_solution_traits<
    facility_location_solution<FacilityCost, Voronoi>> {
    typedef voronoi_traits<Voronoi> VT;
    typedef facility_location_solution<FacilityCost, Voronoi> FLS;

  public:
    typedef typename VT::VertexType VertexType;
    typedef typename VT::DistanceType Dist;
    typedef typename VT::GeneratorsSet ChosenFacilitiesSet;
    /// unchosen facilities set
    typedef puretype(std::declval<FLS>().get_unchosen_facilities())
        UnchosenFacilitiesSet;
};

} // !data_structures
} // !paal

#endif // PAAL_FACILITY_LOCATION_SOLUTION_HPP
