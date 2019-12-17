//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file fusion_algorithms.hpp
 :* @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-02-26
 */
#ifndef PAAL_FUSION_ALGORITHMS_HPP
#define PAAL_FUSION_ALGORITHMS_HPP

#include "paal/utils/type_functions.hpp"

#include <boost/fusion/include/begin.hpp>

#include <functional>

namespace paal {
namespace data_structures {

/**
 * @brief class for polymorphic join on boost fusion sequence
 */
class polymorfic_fold {
    ///Fold
    struct Fold {
        template <typename Functor, typename IterEnd,
                  typename AccumulatorFunctor, typename AccumulatorData>
        typename std::result_of<AccumulatorFunctor(AccumulatorData)>::type
        operator()(Functor, AccumulatorFunctor accumulatorFunctor,
                   AccumulatorData accumulatorData, IterEnd, IterEnd) const {
            return accumulatorFunctor(accumulatorData);
        }

        template <typename Functor, typename IterBegin, typename IterEnd,
                  typename AccumulatorFunctor, typename AccumulatorData,
                  /// this dummy condition is needed because on the lookup phase
                  /// some compilers on some instances (clang-3.4) try to
                  /// instantiate this template
                  /// which causes infinite loop
                  typename Dummy = typename std::enable_if<
                      !std::is_same<IterEnd, IterBegin>::value>::type>
        auto operator()(Functor f, AccumulatorFunctor accumulatorFunctor,
                        AccumulatorData accumulatorData, IterBegin begin,
                        IterEnd end) const
            -> typename std::result_of<Functor(
                typename boost::fusion::result_of::deref<IterBegin>::type,
                AccumulatorFunctor, AccumulatorData,
                // TODO Why this move is needed??? (without it doesn't compile
                // on clang-3.4)
                decltype(std::move(std::bind(
                    *this, f, std::placeholders::_1, std::placeholders::_2,
                    boost::fusion::next(begin), end))))>::type {
            auto continuation = std::bind(*this, f, std::placeholders::_1,
                                          std::placeholders::_2,
                                          boost::fusion::next(begin), end);

            return f(*begin, accumulatorFunctor, accumulatorData, continuation);
        }
    };

  public:
    /**
     * @brief operator()
     *
     * @tparam Functor
     * @tparam AccumulatorFunctor
     * @tparam AccumulatorData
     * @tparam Sequence
     * @param f
     * @param accumulatorFunctor
     * @param accumulatorData
     * @param seq
     *
     * @return
     */
    template <typename Functor, typename AccumulatorFunctor, typename AccumulatorData, typename Sequence>
    auto operator()(Functor f, AccumulatorFunctor accumulatorFunctor,
                    AccumulatorData accumulatorData, Sequence &seq) const{
        return Fold {}
        (f, accumulatorFunctor, accumulatorData, boost::fusion::begin(seq),
         boost::fusion::end(seq));
    }
};

/**
 * @brief Find for StaticLazyJoin
 */
class Satisfy {
    /**
     * @brief satisfy, specialization for empty Join
     *
     * @tparam Predicate
     *
     * @return
     */
    template <typename Predicate, typename IterEnd>
    bool satisfy(Predicate, IterEnd, IterEnd) const {
        return false;
    }

    /**
     * @brief satisfy
     *
     * @tparam Predicate
     * @tparam IterBegin
     * @tparam IterEnd
     * @param pred
     * @param begin
     * @param end
     *
     * @return
     */
    template <typename Predicate, typename IterBegin, typename IterEnd>
    bool satisfy(Predicate pred, IterBegin begin, IterEnd end) const {
        if (pred(*begin)) {
            return true;
        }
        return satisfy(pred, boost::fusion::next(begin), end);
    }

  public:
    /**
     * @brief operator()
     *
     * @tparam Predicate
     * @tparam Seq
     * @param pred
     * @param seq
     *
     * @return
     */
    template <typename Predicate, typename Seq>
    bool operator()(Predicate pred, Seq &seq) const {
        return satisfy(pred, boost::fusion::begin(seq),
                       boost::fusion::end(seq));
    }
};

} //!data_structures
} //!paal

#endif // PAAL_FUSION_ALGORITHMS_HPP
