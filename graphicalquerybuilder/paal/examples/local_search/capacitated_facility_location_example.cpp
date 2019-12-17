//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file capacitated_facility_location_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
    //! [CFL Search Example]
#include "test/test_utils/sample_graph.hpp"

#include "paal/local_search/facility_location/facility_location.hpp"
#include "paal/data_structures/voronoi/capacitated_voronoi.hpp"
#include "paal/utils/functors.hpp"

using namespace paal::local_search;

int main() {
    // sample data
    typedef sample_graphs_metrics SGM;
    auto gm = SGM::get_graph_metric_small();

    std::vector<int> fcostsv{ 7, 8 };
    auto facilityCost = paal::utils::make_array_to_functor(fcostsv);

    std::vector<int> fcapv{ 2, 2 };
    auto facilityCapacity = paal::utils::make_array_to_functor(fcapv);

    std::vector<int> cdemv{ 2, 2, 1, 3, 3 };
    auto clientDemand = paal::utils::make_array_to_functor(cdemv);

    // define voronoi and solution
    typedef paal::data_structures::capacitated_voronoi<
        decltype(gm), decltype(facilityCapacity), decltype(clientDemand)>
        VorType;
    typedef paal::data_structures::facility_location_solution<
        decltype(facilityCost), VorType> Sol;
    typedef paal::data_structures::voronoi_traits<VorType> VT;
    typedef VT::GeneratorsSet GSet;
    typedef VT::VerticesSet VSet;
    typedef Sol::UnchosenFacilitiesSet USet;

    // create voronoi and solution
    VorType voronoi(GSet{ SGM::A },
                    VSet{ SGM::A, SGM::B, SGM::C, SGM::D, SGM::E }, gm,
                    facilityCapacity, clientDemand);
    Sol sol(std::move(voronoi), USet{ SGM::B }, facilityCost);

    // search
    facility_location_first_improving(sol, default_remove_fl_components(),
                                      default_add_fl_components(),
                                      default_swap_fl_components());

    // print result
    auto const &ch = sol.get_chosen_facilities();
    std::copy(ch.begin(), ch.end(), std::ostream_iterator<int>(std::cout, ","));
    std::cout << std::endl;

    return 0;
}
    //! [CFL Search Example]
