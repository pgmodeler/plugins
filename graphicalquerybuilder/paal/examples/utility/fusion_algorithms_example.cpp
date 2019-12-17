//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file fusion_algorithms_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-04
 */

#include "paal/utils/fusion_algorithms.hpp"
#include "paal/utils/infinity.hpp"

#include <boost/fusion/include/vector.hpp>

#include <iostream>

struct F {
    template <class Num, class AccumulatorFunctor, class AccumulatorData,
              class Continuation>
    void operator()(Num num, AccumulatorFunctor accFunctor,
                    AccumulatorData accData, Continuation continuation) const
        // TODO if we  use auto -> decltype(accFunctor(accData))  here instead
        // of void, the code does not compile on g++-4.8
        {
        if (accData < num) {
            auto print = [](Num n) { std::cout << n << std::endl; };
            return continuation(print, num);
        } else {
            return continuation(accFunctor, accData);
        }
    }
};

int main() {
    boost::fusion::vector<int, float, long long> v(12, 5.5f, 2ll);

    paal::data_structures::polymorfic_fold fold{};
    fold(F{}, [](paal::minus_infinity) {
        std::cout << "Empty Collection" << std::endl;
    },
         paal::minus_infinity{}, v);
}
