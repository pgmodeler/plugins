//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file scheduling_jobs.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2013-11-19
 */
#ifndef PAAL_SCHEDULING_JOBS_HPP
#define PAAL_SCHEDULING_JOBS_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE

#include "paal/data_structures/fraction.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/numeric.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

namespace paal {
namespace greedy {

namespace detail {

template <class MachineIterator, class JobIterator, class GetSpeed,
          class GetLoad>
struct sched_traits {
    typedef typename std::iterator_traits<MachineIterator>::reference
        machine_reference;
    typedef typename std::iterator_traits<JobIterator>::reference job_reference;
    typedef pure_result_of_t<GetSpeed(machine_reference)> speed_t;
    typedef pure_result_of_t<GetLoad(job_reference)> load_t;
    typedef data_structures::fraction<load_t, speed_t> frac_t;
};

template <class MachineIterator, class JobIterator, class GetSpeed,
          class GetLoad, class Traits = sched_traits<
                             MachineIterator, JobIterator, GetSpeed, GetLoad>>
typename Traits::frac_t calculate_bound(const MachineIterator mfirst,
                                        const MachineIterator mlast,
                                        const JobIterator jfirst,
                                        const JobIterator jlast,
                                        GetSpeed get_speed, GetLoad get_load) {
    typedef typename Traits::speed_t Speed;
    typedef typename Traits::load_t Load;
    typedef typename Traits::frac_t Frac;

    auto jobs_num = jlast - jfirst;
    auto machines_num = mlast - mfirst;

    std::vector<Speed> speed_sum(machines_num);
    std::transform(mfirst, mlast, speed_sum.begin(), get_speed);
    boost::partial_sum(speed_sum, speed_sum.begin());

    std::vector<Load> load_sum(jobs_num);
    std::transform(jfirst, jlast, load_sum.begin(), get_load);
    boost::partial_sum(load_sum, load_sum.begin());

    typedef decltype(machines_num) MachinesNumType;
    assert(jobs_num > 0 && machines_num > 0);
    Frac result(get_load(*jfirst), get_speed(*mfirst));
    for (auto jobID : irange(jobs_num)) {
        Load load = get_load(jfirst[jobID]);
        auto get_single = [ = ](MachinesNumType i) {
            return Frac(load, get_speed(mfirst[i]));
        };
        auto get_summed = [&](MachinesNumType i) {
            return Frac(load_sum[jobID], speed_sum[i]);
        };
        auto condition = [ = ](MachinesNumType i) {
            return get_summed(i) >= get_single(i);
        };
        auto machines_ids = boost::counting_range(
            static_cast<MachinesNumType>(0), machines_num);
        // current range based version in boost is broken
        // should be replaced when released
        // https://github.com/boostorg/algorithm/pull/4
        auto it = std::partition_point(machines_ids.begin(), machines_ids.end(),
                                       condition);
        MachinesNumType machineID =
            (it != machines_ids.end()) ? *it : machines_num - 1;
        auto getMax = [ = ](MachinesNumType i) {
            return std::max(get_single(i), get_summed(i));
        };

        Frac candidate = getMax(machineID);
        if (machineID != 0) {
            assign_min(candidate, getMax(machineID - 1));
        }
        assign_max(result, candidate);
    }
    return result;
}

template <class MachineIterator, class JobIterator, class OutputIterator,
          class GetSpeed, class GetLoad, class RoundFun>
void schedule(MachineIterator mfirst, MachineIterator mlast, JobIterator jfirst,
              JobIterator jlast, OutputIterator result, GetSpeed get_speed,
              GetLoad get_load, RoundFun round) {
    typedef sched_traits<MachineIterator, JobIterator, GetSpeed, GetLoad>
        Traits;
    typedef typename Traits::speed_t Speed;
    typedef typename Traits::load_t Load;

    if (mfirst == mlast || jfirst == jlast) {
        return;
    }

    std::vector<MachineIterator> machines;
    boost::copy(boost::counting_range(mfirst, mlast),
                std::back_inserter(machines));
    auto get_speed_from_iterator = utils::make_lift_iterator_functor(get_speed);
    boost::sort(machines, utils::make_functor_to_comparator(
                              get_speed_from_iterator, utils::greater{}));

    std::vector<JobIterator> jobs;
    boost::copy(boost::counting_range(jfirst, jlast), std::back_inserter(jobs));
    auto get_load_from_iterator = utils::make_lift_iterator_functor(get_load);
    boost::sort(jobs, utils::make_functor_to_comparator(get_load_from_iterator,
                                                        utils::greater{}));

    auto bound = detail::calculate_bound(
        machines.begin(), machines.end(), jobs.begin(), jobs.end(),
        get_speed_from_iterator, get_load_from_iterator);
    Load bound_load = bound.num;
    Speed bound_speed = bound.den;
    Load current_load{};
    auto emit = [&result](MachineIterator miter, JobIterator jiter) {
        *result = std::make_pair(miter, jiter);
        ++result;
    };
    auto job_iter = jobs.begin();
    for (auto machine_iter = machines.begin(); machine_iter != machines.end();
         ++machine_iter) {
        auto &&machine = *(*machine_iter);
        Speed speed = get_speed(machine);
        while (job_iter != jobs.end()) {
            auto &&job = *(*job_iter);
            Load job_load = get_load(job) * bound_speed,
                 new_load = current_load + job_load;
            assert(new_load <= bound_load * (2 * speed));
            if (bound_load * speed < new_load) {
                Load frac_load = bound_load * speed - current_load;
                if (round(frac_load, job_load)) {
                    emit(*machine_iter, *job_iter);
                } else {
                    auto next_machine_iter = std::next(machine_iter);
                    assert(next_machine_iter != machines.end());
                    emit(*next_machine_iter, *job_iter);
                }
                ++job_iter;
                current_load = job_load - frac_load;
                break;
            }
            emit(*machine_iter, *job_iter);
            ++job_iter;
            current_load = new_load;
        }
    }
    assert(job_iter == jobs.end());
}
} //!detail

/*
 * @brief This is deterministic solve scheduling jobs on machines with different
 * speeds problem and return schedule
 *
 * Example:
 *  \snippet scheduling_jobs_example.cpp Scheduling Jobs Example
 *
 * example file is scheduling_jobs_example.cpp
 *
 * @param mfirst
 * @param mlast
 * @param jfirst
 * @param jlast
 * @param result
 * @param get_speed
 * @param get_load
 * @tparam MachineIterator
 * @tparam JobIterator
 * @tparam OutputIterator
 * @tparam GetSpeed
 * @tparam GetLoad
 */
template <class MachineIterator, class JobIterator, class OutputIterator,
          class GetSpeed, class GetLoad>
void schedule_deterministic(const MachineIterator mfirst,
                            const MachineIterator mlast,
                            const JobIterator jfirst, const JobIterator jlast,
                            OutputIterator result, GetSpeed get_speed,
                            GetLoad get_load) {
    detail::schedule(mfirst, mlast, jfirst, jlast, result, get_speed, get_load,
                     utils::always_true{});
}

/*
 * @brief This is randomized solve scheduling jobs on machines with different
 * speeds problem and return schedule.
 *
 * Example:
 *  \snippet scheduling_jobs_example.cpp Scheduling Jobs Example
 *
 * example file is scheduling_jobs_example.cpp
 *
 * @param mfirst
 * @param mlast
 * @param jfirst
 * @param jlast
 * @param result
 * @param get_speed
 * @param get_load
 * @param gen
 * @tparam MachineIterator
 * @tparam JobIterator
 * @tparam OutputIterator
 * @tparam GetSpeed
 * @tparam GetLoad
 * @tparam RandomNumberGenerator
 */
template <class MachineIterator, class JobIterator, class OutputIterator,
          class GetSpeed, class GetLoad,
          class RandomNumberGenerator = std::default_random_engine>
void schedule_randomized(const MachineIterator mfirst,
                         const MachineIterator mlast, const JobIterator jfirst,
                         const JobIterator jlast, OutputIterator result,
                         GetSpeed get_speed, GetLoad get_load,
                         RandomNumberGenerator &&gen =
                             std::default_random_engine(97345631u)) {
    typedef typename detail::sched_traits<MachineIterator, JobIterator,
                                          GetSpeed, GetLoad> Traits;
    double alpha = std::uniform_real_distribution<double>()(gen);
    auto round = [alpha](typename Traits::load_t fractional_load,
                         typename Traits::load_t total_load) {
        return total_load * alpha < fractional_load;
    };
    detail::schedule(mfirst, mlast, jfirst, jlast, result, get_speed, get_load,
                     round);
}

} //!greedy
} //!paal

#endif // PAAL_SCHEDULING_JOBS_HPP
