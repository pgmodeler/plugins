//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file k_median_solution.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-03-08
 */
#ifndef PAAL_K_MEDIAN_SOLUTION_HPP
#define PAAL_K_MEDIAN_SOLUTION_HPP

#include "paal/utils/functors.hpp"
#include "paal/data_structures/facility_location/facility_location_solution.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief solution for k median problem
 *
 * @tparam voronoiType
 */
template <typename voronoiType>
class k_median_solution : public data_structures::facility_location_solution<
    utils::return_zero_functor, voronoiType> {
    typedef data_structures::facility_location_solution<
        utils::return_zero_functor, voronoiType> base;

  public:
    /**
     * @brief constructor
     *
     * @param voronoi
     * @param uf
     * @param k
     */
    k_median_solution(voronoiType voronoi,
                      typename base::UnchosenFacilitiesSet uf, int k)
        : base(std::move(voronoi), std::move(uf), m_zero_func) {
        assert(int(base::get_chosen_facilities().size()) == k);
    }

  private:
    utils::return_zero_functor m_zero_func;
};

} // data_structures

namespace data_structures {
/**
 * @brief specialization of facility_location_solution_traits
 *
 * @tparam voronoi
 */
template <typename voronoi>
class facility_location_solution_traits<
    data_structures::k_median_solution<voronoi>> {
    typedef voronoi_traits<voronoi> VT;
    typedef data_structures::k_median_solution<voronoi> KMS;

  public:
    typedef typename VT::VertexType VertexType;
    typedef typename VT::DistanceType Dist;
    typedef typename VT::GeneratorsSet ChosenFacilitiesSet;
    /// unchosen facilities set type
    typedef puretype(std::declval<KMS>().get_unchosen_facilities())
        UnchosenFacilitiesSet;
};
}

} // paal

#endif // PAAL_K_MEDIAN_SOLUTION_HPP
