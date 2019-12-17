//=======================================================================
// Copyright (c) 2014 Andrzej Pacuk, Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lsh_nearest_neighbors_regression.hpp
 * @brief
 * @author Andrzej Pacuk, Piotr Wygocki
 * @version 1.0
 * @date 2014-10-06
 */
#ifndef PAAL_LSH_NEAREST_NEIGHBOURS_REGRESSION_HPP
#define PAAL_LSH_NEAREST_NEIGHBOURS_REGRESSION_HPP

#include "paal/data_structures/thread_pool.hpp"
#include "paal/regression/lsh_functions.hpp"
#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/hash.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/unordered_map_serialization.hpp"

#include <boost/range/algorithm/transform.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/size.hpp>
#include <boost/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include <functional>
#include <iterator>
#include <thread>
#include <type_traits>
#include <vector>

namespace paal {

namespace detail {struct lightweight_tag{};}

using default_hash_function_generator = lsh::hamming_hash_function_generator;

/**
 * @brief functor representing tuple of hash functions
 */
template <typename Funs>
class hash_function_tuple {
    Funs m_hash_funs;
    using fun_t = range_to_elem_t<Funs>;

    template <typename Point>
        class apply {
            Point const & m_point;
        public:
            apply(Point const & point) :
                m_point(point) {}

            auto operator()(fun_t const &fun) const -> decltype(fun(m_point))  {
                return fun(m_point);
            }
        };

public:
    ///serialize
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_hash_funs;
    }

    //default constructor, only for serialization purpose
    hash_function_tuple() = default;

    ///constructor
    hash_function_tuple(Funs funs)
        : m_hash_funs(std::move(funs)) {}

    ///operator==
    bool operator==(hash_function_tuple const & other) const {
        return m_hash_funs == other.m_hash_funs;
    }

    ///operator()(), returns vector of hash values
    template <typename Point>
    auto operator()(Point && point) const {
        using hash_result_single = pure_result_of_t<fun_t(Point)>;
        std::vector<hash_result_single> values;

        values.reserve(m_hash_funs.size());
        boost::transform(m_hash_funs, std::back_inserter(values),
                apply<Point>{point});

        return values;
    }

    /// This is the lightweight version of operator().
    /// It is used when the result of this function is not stored.
    template <typename Point>
    auto operator()(Point && point, detail::lightweight_tag) const {
        return m_hash_funs | boost::adaptors::transformed(apply<Point>{point});
    }
};

/**
 * @brief
 *
 * @tparam FunctionGenerator
 */
template <typename FunctionGenerator = default_hash_function_generator>
class hash_function_tuple_generator {
    using fun_t = pure_result_of_t<FunctionGenerator()>;
    using funs_t = std::vector<fun_t>;
    FunctionGenerator m_function_generator;
    unsigned m_hash_functions_per_point;
public:
    /**
     * @brief
     *
     * @param function_generator
     * @param hash_functions_per_point number of hash functions in single tuple
     */
    hash_function_tuple_generator(FunctionGenerator function_generator,
                                  unsigned hash_functions_per_point) :
        m_function_generator(std::forward<FunctionGenerator>(function_generator)),
        m_hash_functions_per_point(hash_functions_per_point) {
    }


    /**
     * @brief
     *
     * @return hash_function_tuple of m_hash_functions_per_point hash functions
     */
    //TODO change to auto, when it starts working
    hash_function_tuple<funs_t> operator()() const {
        funs_t hash_funs;
        hash_funs.reserve(m_hash_functions_per_point);
        std::generate_n(std::back_inserter(hash_funs),
                        m_hash_functions_per_point,
                        std::ref(m_function_generator));

        return hash_function_tuple<funs_t>(std::move(hash_funs));
    }
};

/**
 * @brief
 *
 * @tparam FunctionGenerator
 * @param function_generator functor generating hash functions
 * @param hash_functions_per_point number of hash functions in single tuple
 *
 * @return
 */
template <typename FunctionGenerator>
auto make_hash_function_tuple_generator(FunctionGenerator &&function_generator,
                                        unsigned hash_functions_per_point) {
    return hash_function_tuple_generator<FunctionGenerator>(
                std::forward<FunctionGenerator>(function_generator),
                hash_functions_per_point);
}

namespace detail {

    template <typename Fun, typename Point>
    auto call(Fun const & f, Point &&p, detail::lightweight_tag) {
        return f(std::forward<Point>(p));
    }

    template <typename Function, typename Point>
    auto call(hash_function_tuple<Function> const & f,
              Point &&p, detail::lightweight_tag tag) {
        return f(std::forward<Point>(p), tag);
    }
} //! detail

/**
 * @brief For each test point counts average result of training points with hash
 * equal to test point's hash, basing on Locality-sensitve hashing.
 *
 * Example: <br>
   \snippet lsh_nearest_neighbors_regression_example.cpp LSH Nearest Neighbors Regression Example
 *
 * example file is lsh_nearest_neighbors_regression_example.cpp
 *
 * @tparam HashValue return type of functions generated by LshFunctionGenerator object
 * @tparam LshFunctionGenerator type of functor which generates proper LSH functions
 * @tparam HashForHashValue hash type to be used in hash maps
 */
template <typename HashValue,
          typename LshFun,
          //TODO default value here supposed to be std::hash
          typename HashForHashValue = range_hash>
class lsh_nearest_neighbors_regression {

    //TODO template param TestResultType
    using res_accu_t = average_accumulator<>;
    using map_t = boost::unordered_map<HashValue, res_accu_t, HashForHashValue>;

    ///hash maps containing average result for each hash key
    std::vector<map_t> m_hash_maps;
    ///hash functions
    std::vector<LshFun> m_hashes;

    ///average result of all training points
    average_accumulator<> m_avg;

public:

    ///serialization
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & m_hash_maps;
        ar & m_hashes;
        ar & m_avg;
    }

    ///default constructor, only for serialization purpose
    lsh_nearest_neighbors_regression() = default;

    /**
     * @brief initializes model and trainings model using training points and results
     *
     * @tparam TrainingPoints
     * @tparam TrainingResults
     * @param training_points
     * @param training_results
     * @param passes number of used LSH functions
     * @param lsh_function_generator functor generating proper LSH functions
     * @param threads_count
     */
    template <typename TrainingPoints, typename TrainingResults, typename LshFunctionGenerator>
    lsh_nearest_neighbors_regression(
            TrainingPoints &&training_points, TrainingResults &&training_results,
            unsigned passes,
            LshFunctionGenerator &&lsh_function_generator,
            unsigned threads_count = std::thread::hardware_concurrency()) :
        m_hash_maps(passes) {

        m_hashes.reserve(passes);
        std::generate_n(std::back_inserter(m_hashes), passes,
                    std::ref(lsh_function_generator));

        update(std::forward<TrainingPoints>(training_points),
               std::forward<TrainingResults>(training_results),
               threads_count);
    }

    ///operator==
    bool operator==(lsh_nearest_neighbors_regression const & other) const {
        return m_avg == other.m_avg &&
               m_hashes == other.m_hashes &&
               m_hash_maps == other.m_hash_maps;
    }


    /**
     * @brief trainings model
     *
     * @tparam TrainingPoints
     * @tparam TrainingResults
     * @param training_points
     * @param training_results
     * @param threads_count
     */
    template <typename TrainingPoints, typename TrainingResults>
    void update(TrainingPoints &&training_points, TrainingResults &&training_results,
            unsigned threads_count = std::thread::hardware_concurrency()) {

        thread_pool threads(threads_count);

        threads.post([&](){ compute_avg(training_results);});

        for (auto &&map_and_fun : boost::combine(m_hash_maps, m_hashes)) {
            auto &map = boost::get<0>(map_and_fun);
            //fun is passed by value because of efficiency reasons
            threads.post([&, fun = boost::get<1>(map_and_fun)]() {add_values(fun, map, training_points, training_results);});
        }
        threads.run();
    }

    /**
     * @brief queries model, does not heave threads_count parameter, because this is much more natural
     * to do from outside of the function
     *
     * @tparam TestPoints
     * @tparam OutputIterator
     * @param test_points
     * @param result
     */
    template <typename TestPoints, typename OutputIterator>
    void test(TestPoints &&test_points, OutputIterator result) const {
        assert(!m_avg.empty());

        for (auto &&test_point : test_points) {
            average_accumulator<> avg;
            for(auto && map_and_fun : boost::combine(m_hash_maps, m_hashes)) {
                auto const &map = boost::get<0>(map_and_fun);
                auto const &fun = boost::get<1>(map_and_fun);
                auto got = map.find(detail::call(fun, test_point, detail::lightweight_tag{}),
                                    HashForHashValue{}, utils::equal_to_unspecified{});
                if (got != map.end()) {
                    avg.add_value(got->second.get_average_unsafe());
                }
            }
            *result = avg.get_average(m_avg.get_average());
            ++result;
        }
    }

private:

    ///adds values to one hash map
    template <typename Points, typename Results>
    void add_values(LshFun fun, map_t & map, Points && training_points, Results && training_results) {
        for (auto &&training_point_result : boost::combine(training_points, training_results)) {
            auto && point = boost::get<0>(training_point_result);
            auto && res = boost::get<1>(training_point_result);

            //the return value of this call might be impossible to store in the map
            auto got = map.find(call(fun, point, detail::lightweight_tag{}),
                                 HashForHashValue{}, utils::equal_to_unspecified{});
            if (got != map.end()) {
                got->second.add_value(res);
            } else {
                map[fun(point)].add_value(res);
            }

        }
    }

    ///computes average
    template <typename Results>
    void compute_avg(Results const & training_results) {
        for (auto && res :training_results) {
            m_avg.add_value(res);
        }
    }
};

/**
 * @brief this is the most general version of the make_lsh_nearest_neighbors_regression,
 *        It takes any hash function generator.
 *
 * @tparam TrainingPoints
 * @tparam TrainingResults
 * @tparam LshFunctionGenerator
 * @param training_points
 * @param training_results
 * @param passes number of used LSH functions
 * @param lsh_function_generator functor generating proper LSH functions
 * @param threads_count
 *
 * @return lsh_nearest_neighbors_regression model
 */

template <typename TrainingPoints, typename TrainingResults,
          typename LshFunctionGenerator>
auto make_lsh_nearest_neighbors_regression(
             TrainingPoints &&training_points, TrainingResults &&training_results,
             unsigned passes,
             LshFunctionGenerator &&lsh_function_generator,
             unsigned threads_count = std::thread::hardware_concurrency()) {
    using lsh_fun = pure_result_of_t<LshFunctionGenerator()>;
    using hash_result = typename std::remove_reference<
        typename std::result_of<lsh_fun(
                range_to_ref_t<TrainingPoints>
                )>::type
        >::type;

    return lsh_nearest_neighbors_regression<hash_result, lsh_fun>(
            std::forward<TrainingPoints>(training_points),
            std::forward<TrainingResults>(training_results),
            passes,
            std::forward<LshFunctionGenerator>(lsh_function_generator),
            threads_count);
}


/**
 * @brief This is the special version  of make_lsh_nearest_neighbors_regression.
 *        This version assumes that hash function is concatenation (tuple) of several hash functions.
 *        In this function user provide Function generator for the inner functions only.
 *
 * @tparam TrainingPoints
 * @tparam TrainingResults
 * @tparam FunctionGenerator
 * @param training_points
 * @param training_results
 * @param passes
 * @param function_generator
 * @param hash_functions_per_point
 * @param threads_count
 *
 * @return
 */
template <typename TrainingPoints, typename TrainingResults,
          typename FunctionGenerator>
auto make_lsh_nearest_neighbors_regression_tuple_hash(
             TrainingPoints &&training_points, TrainingResults &&training_results,
             unsigned passes,
             FunctionGenerator &&function_generator,
             unsigned hash_functions_per_point,
             unsigned threads_count = std::thread::hardware_concurrency()) {

    auto tuple_lsh = paal::make_hash_function_tuple_generator(
                    std::forward<FunctionGenerator>(function_generator),
                    hash_functions_per_point);
    return make_lsh_nearest_neighbors_regression(
            std::forward<TrainingPoints>(training_points),
            std::forward<TrainingResults>(training_results),
            passes,
            std::move(tuple_lsh),
            threads_count);
}

} //! paal

#endif // PAAL_LSH_NEAREST_NEIGHBOURS_REGRESSION_HPP
