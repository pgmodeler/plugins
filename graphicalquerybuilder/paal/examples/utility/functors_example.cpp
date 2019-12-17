//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file functors_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date
 */

#include "paal/utils/functors.hpp"

#include <vector>
#include <string>

using namespace paal::utils;

void functors_example() {
    // skip
    skip_functor skip;

    skip(2, 2.1, "asda");

    // identity
    identity_functor id;

    assert(id("asd") == std::string("asd"));
    assert(id(7) == 7);

    // return something
    always_true retTrue;
    always_false retFalse;
    return_zero_functor retZero;

    assert(retTrue(2, 2.3, "abc"));
    assert(!retFalse(2, 2.3, "abc"));
    assert(retZero(2, 2.3, "abc") == 0);

    // assert
    // assert_functor assertFun;
    // assertFun(); //aborts

    // array to functor
    std::vector<int> vec{ 1, 2, 3 };
    auto vecFun = make_array_to_functor(vec);
    assert(vecFun(1) == 2);

    auto vecFunWithOffset = make_array_to_functor(vec, 1);
    assert(vecFunWithOffset(1) == 3);
}

void compare_functors() {
    greater g;
    greater_equal ge;
    less l;
    less_equal le;
    equal_to e;
    not_equal_to ne;

    assert(!g(1, 2));
    assert(!g(1, 1));
    assert(g(2, 1));

    assert(!ge(1, 2));
    assert(ge(1, 1));
    assert(ge(2, 1));

    assert(l(1, 2));
    assert(!l(1, 1));
    assert(!l(2, 1));

    assert(le(1, 2));
    assert(le(1, 1));
    assert(!le(2, 1));

    assert(!e(1, 2));
    assert(e(1, 1));
    assert(!e(2, 1));

    assert(ne(1, 2));
    assert(!ne(1, 1));
    assert(ne(2, 1));
}

void comparator_functor() {
    auto getFirst = [](std::pair<int, int> p) { return p.first; };
    auto compareFirst = make_functor_to_comparator(getFirst);

    assert(!compareFirst(std::make_pair(1, 2), std::make_pair(0, 1)));

    auto compareFirstDesc = make_functor_to_comparator(getFirst, greater());

    assert(compareFirstDesc(std::make_pair(1, 2), std::make_pair(0, 1)));
}

void boolean_functors() {
    Not notFun;
    Or orFun;
    And andFun;

    assert(!notFun(true));
    assert(notFun(false));

    assert(!orFun(false, false));
    assert(orFun(true, false));
    assert(orFun(false, true));
    assert(orFun(true, true));

    assert(!andFun(false, false));
    assert(!andFun(true, false));
    assert(!andFun(false, true));
    assert(andFun(true, true));
}

void lift_operator_functor() {
    auto oper = [](int a, int b) { return a + b > 0; };
    return_zero_functor zero;
    return_constant_functor<int, 5> five;

    auto f = make_lift_binary_operator_functor(zero, five, oper);

    assert(f(1, 2, 41243.2, "dada"));
}

void boolean_functors_on_functors() {
    always_true retTrue;
    always_false retFalse;

    {
        auto trueFunctor = make_not_functor(retFalse);
        assert(trueFunctor(1.2, "xada", 3));
    }

    {
        auto falseFunctor = make_not_functor(retTrue);
        assert(!falseFunctor(1.2, "xada", 3));
    }

    {
        auto trueFunctor = make_or_functor(retTrue, retFalse);
        assert(trueFunctor(1.2, "xada", 3));
    }

    {
        auto falseFunctor = make_and_functor(retTrue, retFalse);
        assert(!falseFunctor(1.2, "xada", 3));
    }

    {
        auto trueFunctor = make_xor_functor(retTrue, retFalse);
        assert(trueFunctor(1.2, "xada", 3));
    }
}

int main() {
    functors_example();
    boolean_functors();
    comparator_functor();
    boolean_functors();
    lift_operator_functor();
    boolean_functors_on_functors();

    return 0;
}
