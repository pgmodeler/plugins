//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file facility_location_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */
    //! [FL Search Example]
#include "test/test_utils/sample_graph.hpp"

#include "paal/local_search/facility_location/facility_location.hpp"
#include "paal/utils/functors.hpp"

#include <boost/range/algorithm/copy.hpp>

#include <iostream>

using namespace paal::local_search;

int main() {


    // sample data
    typedef sample_graphs_metrics SGM;
    auto gm = SGM::get_graph_metric_small();
    std::vector<int> fcosts{ 7, 8 };
    typedef paal::utils::array_to_functor<std::vector<int>> Cost;

    // define voronoi and solution
    typedef paal::data_structures::voronoi<decltype(gm)> VorType;
    typedef paal::data_structures::facility_location_solution<Cost, VorType>
        Sol;
    typedef paal::data_structures::voronoi_traits<VorType> VT;
    typedef VT::GeneratorsSet GSet;
    typedef VT::VerticesSet VSet;
    typedef Sol::UnchosenFacilitiesSet USet;

    // create voronoi and solution
    VorType voronoi(GSet{}, VSet{ SGM::A, SGM::B, SGM::C, SGM::D, SGM::E }, gm);
    Sol sol(std::move(voronoi), USet{ SGM::A, SGM::B },
            paal::utils::make_array_to_functor(fcosts));

    // create facility location local search components
    default_remove_fl_components rem;
    default_add_fl_components add;
    default_swap_fl_components swap;

    // search
    facility_location_first_improving(sol, rem, add, swap);

    // print result
    boost::copy(sol.get_chosen_facilities(), std::ostream_iterator<int>(std::cout, ","));
    std::cout << std::endl;

    return true;
}
    //! [FL Search Example]
