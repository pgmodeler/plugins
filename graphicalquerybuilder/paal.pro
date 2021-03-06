######################################################################
# Automatically generated by qmake (3.1) Sun Jun 23 17:22:06 2019
######################################################################


include(../pgmodeler.pri)

TEMPLATE = lib
TARGET = pgmodeler

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += docs/namespaces.hpp \
           test/data_structures/cycle.hpp \
           test/iterative_rounding/log_visitor.hpp \
           test/test_utils/2_local_search_logger.hpp \
           test/test_utils/auctions.hpp \
           test/test_utils/budgeted_maximum_coverage_check.hpp \
           test/test_utils/fractional_winner_determination_in_MUCA_test_utils.hpp \
           test/test_utils/get_test_dir.hpp \
           test/test_utils/knapsack_tags_utils.hpp \
           test/test_utils/logger.hpp \
           test/test_utils/read_bounded_deg_mst.hpp \
           test/test_utils/read_dist.hpp \
           test/test_utils/read_gen_ass.hpp \
           test/test_utils/read_knapsack.hpp \
           test/test_utils/read_lp.hpp \
           test/test_utils/read_orlib_fl.hpp \
           test/test_utils/read_orlib_km.hpp \
           test/test_utils/read_orlib_sc.hpp \
           test/test_utils/read_ss.hpp \
           test/test_utils/read_steinlib.hpp \
           test/test_utils/read_tsplib.hpp \
           test/test_utils/read_two_dimensional_data.hpp \
           test/test_utils/sample_graph.hpp \
           test/test_utils/scheduling.hpp \
           test/test_utils/serialization.hpp \
           test/test_utils/set_cover_check.hpp \
           test/test_utils/simple_single_local_search_components.hpp \
           test/test_utils/sketch_accuracy_check.hpp \
           test/test_utils/system.hpp \
           test/test_utils/test_gamma_oracle_xor_bids.hpp \
           test/test_utils/test_result_check.hpp \
           test/test_utils/winner_determination_in_MUCA_long_test.hpp \
           test/test_utils/winner_determination_in_MUCA_test_utils.hpp \
           include/paal/auctions/auction_components.hpp \
           include/paal/auctions/auction_traits.hpp \
           include/paal/auctions/auction_utils.hpp \
           include/paal/auctions/single_minded_auctions.hpp \
           include/paal/auctions/xor_bids.hpp \
           include/paal/clustering/k_means_clustering.hpp \
           include/paal/clustering/k_means_clustering_engine.hpp \
           include/paal/data_structures/bimap.hpp \
           include/paal/data_structures/bimap_traits.hpp \
           include/paal/data_structures/collection_starts_from_last_change.hpp \
           include/paal/data_structures/combine_iterator.hpp \
           include/paal/data_structures/cycle_iterator.hpp \
           include/paal/data_structures/fraction.hpp \
           include/paal/data_structures/mapped_file.hpp \
           include/paal/data_structures/object_with_copy.hpp \
           include/paal/data_structures/splay_tree.hpp \
           include/paal/data_structures/stack.hpp \
           include/paal/data_structures/subset_iterator.hpp \
           include/paal/data_structures/thread_pool.hpp \
           include/paal/data_structures/ublas_traits.hpp \
           include/paal/data_structures/vertex_to_edge_iterator.hpp \
           include/paal/dynamic/knapsack_0_1.hpp \
           include/paal/dynamic/knapsack_0_1_fptas.hpp \
           include/paal/dynamic/knapsack_unbounded.hpp \
           include/paal/dynamic/knapsack_unbounded_fptas.hpp \
           include/paal/greedy/knapsack_0_1_two_app.hpp \
           include/paal/greedy/knapsack_unbounded_two_app.hpp \
           include/paal/greedy/steiner_tree_greedy.hpp \
           include/paal/iterative_rounding/ir_components.hpp \
           include/paal/iterative_rounding/iterative_rounding.hpp \
           include/paal/iterative_rounding/min_cut.hpp \
           include/paal/local_search/custom_components.hpp \
           include/paal/local_search/local_search.hpp \
           include/paal/local_search/local_search_concepts.hpp \
           include/paal/local_search/local_search_obj_function.hpp \
           include/paal/local_search/search_components.hpp \
           include/paal/local_search/search_obj_func_components.hpp \
           include/paal/local_search/search_traits.hpp \
           include/paal/local_search/simulated_annealing.hpp \
           include/paal/local_search/trivial_solution_commit.hpp \
           include/paal/lp/constraints.hpp \
           include/paal/lp/expressions.hpp \
           include/paal/lp/glp.hpp \
           include/paal/lp/ids.hpp \
           include/paal/lp/lp_base.hpp \
           include/paal/lp/lp_row_generation.hpp \
           include/paal/lp/problem_type.hpp \
           include/paal/multiway_cut/multiway_cut.hpp \
           include/paal/regression/lsh_functions.hpp \
           include/paal/regression/lsh_nearest_neighbors_regression.hpp \
           include/paal/sketch/frequent_directions.hpp \
           include/paal/steiner_tree/dreyfus_wagner.hpp \
           include/paal/steiner_tree/zelikovsky_11_per_6.hpp \
           include/paal/utils/accumulate_functors.hpp \
           include/paal/utils/assign_updates.hpp \
           include/paal/utils/concepts.hpp \
           include/paal/utils/contract_bgl_adjacency_matrix.hpp \
           include/paal/utils/fast_exp.hpp \
           include/paal/utils/floating.hpp \
           include/paal/utils/functors.hpp \
           include/paal/utils/fusion_algorithms.hpp \
           include/paal/utils/hash.hpp \
           include/paal/utils/infinity.hpp \
           include/paal/utils/irange.hpp \
           include/paal/utils/iterator_utils.hpp \
           include/paal/utils/knapsack_utils.hpp \
           include/paal/utils/less_pointees.hpp \
           include/paal/utils/make.hpp \
           include/paal/utils/make_tuple.hpp \
           include/paal/utils/parse_file.hpp \
           include/paal/utils/performance_measures.hpp \
           include/paal/utils/pretty_stream.hpp \
           include/paal/utils/print_collection.hpp \
           include/paal/utils/property_map.hpp \
           include/paal/utils/read_rows.hpp \
           include/paal/utils/read_svm.hpp \
           include/paal/utils/rotate.hpp \
           include/paal/utils/singleton_iterator.hpp \
           include/paal/utils/system_message.hpp \
           include/paal/utils/type_functions.hpp \
           include/paal/utils/unordered_map_serialization.hpp \
           test/greedy/k_center/in_balls.hpp \
           include/paal/auctions/fractional_winner_determination_in_MUCA/fractional_winner_determination_in_MUCA.hpp \
           include/paal/auctions/winner_determination_in_MUCA/winner_determination_in_MUCA.hpp \
           include/paal/data_structures/components/component_traits.hpp \
           include/paal/data_structures/components/components.hpp \
           include/paal/data_structures/components/components_join.hpp \
           include/paal/data_structures/components/components_replace.hpp \
           include/paal/data_structures/components/types_vector.hpp \
           include/paal/data_structures/cycle/cycle_algo.hpp \
           include/paal/data_structures/cycle/cycle_concept.hpp \
           include/paal/data_structures/cycle/cycle_start_from_last_change.hpp \
           include/paal/data_structures/cycle/cycle_traits.hpp \
           include/paal/data_structures/cycle/simple_cycle.hpp \
           include/paal/data_structures/cycle/splay_cycle.hpp \
           include/paal/data_structures/facility_location/facility_location_solution.hpp \
           include/paal/data_structures/facility_location/facility_location_solution_traits.hpp \
           include/paal/data_structures/facility_location/fl_algo.hpp \
           include/paal/data_structures/facility_location/k_median_solution.hpp \
           include/paal/data_structures/metric/basic_metrics.hpp \
           include/paal/data_structures/metric/euclidean_metric.hpp \
           include/paal/data_structures/metric/graph_metrics.hpp \
           include/paal/data_structures/metric/metric_on_idx.hpp \
           include/paal/data_structures/metric/metric_to_bgl.hpp \
           include/paal/data_structures/metric/metric_traits.hpp \
           include/paal/data_structures/tabu_list/tabu_list.hpp \
           include/paal/data_structures/voronoi/capacitated_voronoi.hpp \
           include/paal/data_structures/voronoi/voronoi.hpp \
           include/paal/data_structures/voronoi/voronoi_traits.hpp \
           include/paal/distance_oracle/vertex_vertex/thorup_2kminus1.hpp \
           include/paal/dynamic/knapsack/fill_knapsack_dynamic_table.hpp \
           include/paal/dynamic/knapsack/get_bound.hpp \
           include/paal/dynamic/knapsack/knapsack_common.hpp \
           include/paal/dynamic/knapsack/knapsack_fptas_common.hpp \
           include/paal/greedy/k_center/k_center.hpp \
           include/paal/greedy/k_cut/k_cut.hpp \
           include/paal/greedy/knapsack/knapsack_greedy.hpp \
           include/paal/greedy/scheduling_jobs/scheduling_jobs.hpp \
           include/paal/greedy/scheduling_jobs_on_identical_parallel_machines/scheduling_jobs_on_identical_parallel_machines.hpp \
           include/paal/greedy/scheduling_jobs_with_deadlines_on_a_single_machine/scheduling_jobs_with_deadlines_on_a_single_machine.hpp \
           include/paal/greedy/set_cover/budgeted_maximum_coverage.hpp \
           include/paal/greedy/set_cover/maximum_coverage.hpp \
           include/paal/greedy/set_cover/set_cover.hpp \
           include/paal/greedy/shortest_superstring/prefix_tree.hpp \
           include/paal/greedy/shortest_superstring/shortest_superstring.hpp \
           include/paal/iterative_rounding/bounded_degree_min_spanning_tree/bounded_degree_mst.hpp \
           include/paal/iterative_rounding/bounded_degree_min_spanning_tree/bounded_degree_mst_oracle.hpp \
           include/paal/iterative_rounding/generalised_assignment/generalised_assignment.hpp \
           include/paal/iterative_rounding/steiner_network/prune_restrictions_to_tree.hpp \
           include/paal/iterative_rounding/steiner_network/steiner_network.hpp \
           include/paal/iterative_rounding/steiner_network/steiner_network_oracle.hpp \
           include/paal/iterative_rounding/steiner_tree/steiner_component.hpp \
           include/paal/iterative_rounding/steiner_tree/steiner_components.hpp \
           include/paal/iterative_rounding/steiner_tree/steiner_strategy.hpp \
           include/paal/iterative_rounding/steiner_tree/steiner_tree.hpp \
           include/paal/iterative_rounding/steiner_tree/steiner_tree_oracle.hpp \
           include/paal/iterative_rounding/steiner_tree/steiner_utils.hpp \
           include/paal/iterative_rounding/treeaug/tree_augmentation.hpp \
           include/paal/local_search/2_local_search/2_local_search.hpp \
           include/paal/local_search/2_local_search/2_local_search_components.hpp \
           include/paal/local_search/2_local_search/2_local_search_solution_adapter.hpp \
           include/paal/local_search/facility_location/facility_location.hpp \
           include/paal/local_search/facility_location/facility_location_add.hpp \
           include/paal/local_search/facility_location/facility_location_remove.hpp \
           include/paal/local_search/facility_location/facility_location_solution_adapter.hpp \
           include/paal/local_search/facility_location/facility_location_swap.hpp \
           include/paal/local_search/k_median/k_median.hpp \
           include/paal/local_search/n_queens/n_queens_components.hpp \
           include/paal/local_search/n_queens/n_queens_local_search.hpp \
           include/paal/local_search/n_queens/n_queens_solution.hpp \
           include/paal/utils/algorithms/subset_backtrack.hpp \
           include/paal/utils/algorithms/suffix_array/lcp.hpp \
           include/paal/utils/algorithms/suffix_array/suffix_array.hpp

INCLUDEPATH += $$PWD/include \
               $$PWD/boost

DEPENDPATH += $$PWD/include \
              $$PWD/boost

