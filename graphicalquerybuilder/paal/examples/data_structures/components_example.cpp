//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file components_example.cpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-04
 */

#include "paal/data_structures/components/components.hpp"
#include "paal/data_structures/components/components_replace.hpp"

#include <type_traits>
#include <functional>
#include <cassert>

// this are names for components used in all of the examples
namespace names {
struct A;
struct B;
struct C;
}

namespace ds = paal::data_structures;

// definition of Comps with names names::A, names::B, names::C
template <typename... Args>
using Comps =
    typename ds::components<names::A, names::B, names::C>::type<Args...>;

void example_default_constructor_and_basic_usage() {
    // constructor has default agruments
    typedef Comps<int, double, int> MyComps;
    MyComps comps;
    // getter
    assert(comps.get<names::A>() == 0); // zero initialization is guaranteed

    // setter
    comps.set<names::C>(7);
    assert(comps.get<names::C>() == 7);

    const MyComps &constAlias = comps;
    // const version of get
    constAlias.get<names::B>();
}

void example_constructor_with_all_arguments() {
    // constructor with parameters
    Comps<int, double, int> comps(5, 4, 3);
    assert(comps.get<names::A>() == 5);
    assert(comps.get<names::B>() == 4);
    assert(comps.get<names::C>() == 3);
}

void example_constructor_with_some_arguments() {
    // default value for 3rd argument
    Comps<int, double, int> comps(5, 4);
    assert(comps.get<names::A>() == 5);
    assert(comps.get<names::B>() == 4);
}

int f(int i) { return i; }
void example_calling_function_from_components() {
    // declaration components with function
    typedef Comps<int (*)(int), int (*)(int), int> CompsF;

    // definition
    CompsF comps(f, f, 17);
    const CompsF &constAlias = comps;

    // call the first argument
    assert(comps.call<names::A>(2) == 2);

    // const version
    assert(constAlias.call<names::B>(2) == 2);
}

void example_replacing() {
    // components with replaceped type
    typedef Comps<int (*)(int), double, int> CompsF;

    typedef ds::replaced_type<names::A, std::pair<int, int>, CompsF>::type
        Replaced;
    typedef Comps<std::pair<int, int>, double, int> ReplacedCheck;
    static_assert(std::is_same<Replaced, ReplacedCheck>::value,
                  "Invalid replaceped type");

    // replace components
    CompsF comps(f, 2, 17);
    Replaced replaced = ds::replace<names::A>(std::make_pair(11, 12), comps);

    auto p = replaced.get<names::A>();
    assert(p.first == 11);
    assert(p.second == 12);
    assert(replaced.get<names::B>() == 2);
    assert(replaced.get<names::C>() == 17);
}

// this three function are not default constructible
// this is to show that this is not needed if not explicyty used
struct X {
    explicit X(int _x) : x(_x) {}
    X() = delete;
    bool operator==(X xx) const { return x == xx.x; }

    int x;
};

struct Y {
    explicit Y(int _y) : y(_y) {}
    Y() = delete;

    int y;
};

struct Z {
    explicit Z(int _z) : z(_z) {}
    Z() = delete;

    int z;
};

// comnponts with defaults
template <typename... Args>
using CompsWithDefaults = typename ds::components<
    names::A, names::B, ds::NameWithDefault<names::C, X>>::type<Args...>;

void example_default_parameters() {
    // normal definition
    CompsWithDefaults<int, double, float> compsDef;

    // definition with default 3rd template parameter
    CompsWithDefaults<int, double> compsDef2(1, 2, 3);
    assert(compsDef2.get<names::C>() == X(3));

    // This won't compile
    // CompsWithDefaults<int> comps3;
}

// definition of Comps with names names::A, names::B
template <typename... Args>
using CompsToReplace =
    typename ds::components<names::A, names::B>::type<Args...>;

void example_replacing_struct_without_default_constructors() {
    X x(1);
    Y y(2);
    Z z(3);

    CompsToReplace<X, Y> compsToReplace(x, y);
    // replace, X, Y, Z doesn't have default constructors
    auto s = ds::replace<names::A>(z, compsToReplace);
    assert(s.get<names::A>().z == 3);

    auto s2 = ds::replace<names::B>(z, compsToReplace);
    assert(s2.get<names::B>().z == 3);
}

void example_create_using_make() {
    // constructing objects providing names for objects
    typedef Comps<int, double, float> SomeComps;
    auto someComps = SomeComps::make<names::A, names::C>(1, 2.f);
    assert(someComps.get<names::A>() == 1);
    assert(someComps.get<names::C>() == 2.f);

    auto someComps2 = SomeComps::make<names::C, names::A>(1.f, 2);
    assert(someComps2.get<names::C>() == 1.f);
    assert(someComps2.get<names::A>() == 2);
}

void example_create_using_copy_tag() {
    typedef Comps<int, double, float> SomeComps;
    SomeComps someComps(CompsToReplace<int, int>(1, 2), ds::copy_tag());
    assert(someComps.get<names::A>() == 1);
    assert(someComps.get<names::B>() == 2.);
}

void example_references() {
    // references works also
    int a;
    typedef Comps<int, const int &, int &> CompsWithRefs;
    CompsWithRefs compsWithRefs(a, a, a);

    CompsWithRefs::make<names::B, names::C>(a, a);
}

void example_make_components() {
    // this is the way of constructing components without providing actual types
    typedef ds::components<names::A, names::B> MComps;

    int a;
    auto mComps = MComps::make_components(1, std::ref(a));
    static_assert(std::is_same<std::remove_reference<decltype(mComps)>::type,
                               MComps::type<int, int &>>::value,
                  "Ups...");
}

// this shouldn't compile
// template <typename... Args>
// using CompsWithDefaultsIncorrect = typename  ds::components<
//        names::A, ds::NameWithDefault<names::B, X>, names::C>::type<Args...>;

int main() {
    example_default_constructor_and_basic_usage();
    example_constructor_with_all_arguments();
    example_constructor_with_some_arguments();
    example_calling_function_from_components();
    example_replacing();
    example_default_parameters();
    example_replacing_struct_without_default_constructors();
    example_create_using_make();
    example_create_using_copy_tag();
    example_references();
    example_make_components();
}
