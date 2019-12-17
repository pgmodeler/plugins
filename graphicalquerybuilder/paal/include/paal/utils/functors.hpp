//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file functors.hpp
 * @brief This file contains set of simple useful functors or functor adapters.
 * @author Piotr Wygocki, Robert Rosolek, Andrzej Pacuk
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_FUNCTORS_HPP
#define PAAL_FUNCTORS_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE

#include "paal/utils/type_functions.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <tuple>
#include <utility>

namespace paal {
namespace utils {

// TODO this will not be needed once C++14 auto lambda parameters become available.
/**
 *  @brief Type that can be constructed from anything and has no other functionality
 */
struct ignore_param {
    /**
     * @brief constructor
     *
     * @tparam Args
     */
    template <class... Args>
        ignore_param(Args&&...) {}
};

/**
 * @brief Functor does nothing
 */
struct skip_functor {
    /**
     * @brief operator
     *
     * @tparam Args
     * @param args
     */
    template <typename... Args> void operator()(Args &&... args) const {}
};

/**
 * @brief Functor returns always the same number. The number has to be known at
* compile time
 *
 * @tparam T type of returned value
 * @tparam t return value
 */
template <typename T, T t> struct return_constant_functor {
    /**
     * @brief operator
     *
     * @tparam Args
     * @param args
     *
     * @return
     */
    template <typename... Args> T operator()(Args &&... args) const {
        return t;
    }
};

/**
 * @brief Functor returns always the same number (dynamic version).
 *
 * @tparam T type of returned value
 */
template <typename T> struct dynamic_return_constant_functor {
    /**
     * @brief constructor
     *
     * @param t
     */
    dynamic_return_constant_functor(T t) : m_t(t) {}

    /**
     * @brief operator
     *
     * @tparam Args
     * @param args
     *
     * @return
     */
    template <typename... Args> T operator()(Args &&... args) const {
        return m_t;
    }

  private:
    T m_t;
};

/**
 * @brief make function for dynamic_return_constant_functor
 *
 * @tparam T
 * @param t
 *
 * @return
 */
template <typename T>
auto make_dynamic_return_constant_functor(T t) {
    return dynamic_return_constant_functor<T>(t);
}

/**
 * @brief functor returns its argument
 */
struct identity_functor {
    /**
     * @brief operator()
     *
     * @tparam Arg
     * @param arg
     *
     * @return
     */
    template <typename Arg> auto operator()(Arg &&arg) const->Arg {
        return std::forward<Arg>(arg);
    }
};

/**
 * @brief functor composition: x -> f(g(x))
 *
 * @tparam F
 * @tparam G
 * @param f
 * @param g
 *
 * @return
 */
template <typename F, typename G>
auto compose(F f, G g) {
   return std::bind(f, std::bind(g, std::placeholders::_1));
}

/**
 * @brief functor composition, variadic case
 *
 * @tparam F
 * @tparam Fs
 * @param f
 * @param fs
 *
 * @return
 */
template <typename F, typename... Fs>
auto compose(F f, Fs... fs) {
   return compose(f, compose(fs...));
}

namespace detail {
   /// necessary for ADL to work for get
   template <typename T> void get(T t) {}
}

// TODO make it work also with std::tuple, for that use ADL trick for get
// function and branch compile time between boost::tuples::length and
// std::tuple_size.
/**
 * @brief transforms a functor taking multiple parameters into a functor
 * taking a tuple parameter.
 * @tparam F
 */
template <class F>
class tuple_uncurry {
   F m_f;
   public:
   /**
    * @brief constructor
    *
    * @param f
    */
      tuple_uncurry(F f) : m_f(f) {}

      /**
       * @brief operator
       *
       * @tparam Tuple
       * @param t
       */
      template <class Tuple>
      auto operator()(Tuple&& t) const {
         using detail::get;
         return m_f(get<0>(t), get<1>(t));
      }
};

/**
 * @brief make for tuple_uncurry
 *
 * @tparam F
 * @param f
 *
 * @return
 */
template <class F> auto make_tuple_uncurry(F f) { return tuple_uncurry<F>(f); }

/**
 * @brief functor return false
 */
struct always_false : public return_constant_functor<bool, false> {};

/**
 * @brief functor return true
 */
struct always_true : public return_constant_functor<bool, true> {};

/**
 * @brief functor returns 0
 */
struct return_zero_functor : public return_constant_functor<int, 0> {};

/**
 * @brief functor returns 1
 */
struct return_one_functor : public return_constant_functor<int, 1> {};

/**
 * @brief functors calls assert(false).
 */
struct assert_functor {
    /**
     * @brief operator()
     *
     * @tparam Args
     * @param args
     */
    template <typename... Args> void operator()(Args &&... args) const {
        assert(false);
    }
};

/**
 * @brief removes reference
 */
struct remove_reference {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param t
     *
     * @return
     */
    template <typename T> T operator()(const T &t) const { return t; }
};

/**
 * @brief Counts number of calls
 *
 * @tparam Functor
 * @tparam CounterType
 */
template <typename Functor, typename CounterType = int>
class counting_functor_adaptor {
  public:
    /**
     * @brief Constructor
     *
     * @param cnt count reference
     * @param f functor
     */
    counting_functor_adaptor(Functor f, CounterType &cnt)
        : m_cnt(&cnt), m_functor(std::move(f)) {}

    /**
     * @brief increments the counter and checks if the given limit is reached.
     *
     * @tparam Args
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args)
        ->decltype(std::declval<Functor>()(std::forward<Args>(args)...)) {
        ++(*m_cnt);
        return m_functor(std::forward<Args>(args)...);
    }

  private:
    CounterType *m_cnt;
    Functor m_functor;
};

/**
 * @brief make function for counting_functor_adaptor
 *
 * @tparam CounterType
 * @tparam Functor
 * @param f
 * @param cnt
 *
 * @return
 */
template <typename CounterType = int, typename Functor>
auto make_counting_functor_adaptor(Functor f, CounterType &cnt) {
    return counting_functor_adaptor<Functor, CounterType>(std::move(f), cnt);
}

/**
 * @brief Adapts array as function, providing operator()().
 *
 * @tparam Array
 */
template <typename Array> class array_to_functor {
  public:
    /**
     * @brief constructor
     *
     * @param array
     * @param offset
     */
    array_to_functor(const Array &array, int offset = 0)
        : m_array(array), m_offset(offset) {}

    /// Value type
    typedef decltype(std::declval<const Array>()[0]) Value;

    /**
     * @brief operator
     *
     * @param a
     *
     * @return
     */
    Value operator()(int a) const { return m_array.get()[a + m_offset]; }

  private:
    std::reference_wrapper<const Array> m_array;
    int m_offset;
};

/**
 * @brief make function for array_to_functor
 *
 * @tparam Array
 * @param a
 * @param offset
 *
 * @return
 */
template <typename Array>
auto make_array_to_functor(const Array &a, int offset = 0) {
    return array_to_functor<Array>(a, offset);
}

/**
 * @brief  Wrapper around a functor which adds assigmnent operator as well as
* default constructor.
 * Note, this struct might be dangerous. Using this struct correctly requires
* the underlying
 * functor to live at least as long as this wrapper.
 *
 * @tparam Functor
 */
template <typename Functor> struct assignable_functor {
    /**
     * @brief constructor
     *
     * @param f
     */
    assignable_functor(Functor const &f) : m_f(f) {}
    assignable_functor() = default;

    /**
     * @brief assign operator
     *
     * @param f
     *
     * @return
     */
    assignable_functor &operator=(Functor &f) { m_f = f; }

    /**
     * @brief operator()
     *
     * @tparam Args
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args) const->decltype(
        std::declval<Functor>()(std::forward<Args>(args)...)) {
        return m_f.get()(std::forward<Args>(args)...);
    }

  private:
    std::reference_wrapper<const Functor> m_f;
};

/**
 * @brief make function for assignable_functor
 *
 * @tparam Functor
 * @param f
 *
 * @return
 */
template <typename Functor>
auto make_assignable_functor(Functor &f) {
    return assignable_functor<Functor>(f);
}

/**
 * @brief For given functor f, lift_iterator_functor provides
* operator()(Iterator iterator)
 * which returns f(*iter).
 *
 * @tparam Functor
 */
template <typename Functor> struct lift_iterator_functor {
    /**
     * @brief constructor
     *
     * @param f
     */
    lift_iterator_functor(Functor f) : m_f(std::move(f)) {}

    /**
     * @brief operator()
     *
     * @tparam Iterator
     * @param iter
     *
     * @return
     */
    template <typename Iterator>
    auto operator()(Iterator iter) const->decltype(
        std::declval<Functor>()(*iter)) {
        return m_f(*iter);
    }

  private:
    Functor m_f;
};

/**
 * @brief make function for lift_iterator_functor
 *
 * @tparam Functor
 * @param f
 *
 * @return
 */
template <typename Functor>
auto make_lift_iterator_functor(Functor f) {
    return lift_iterator_functor<Functor>(f);
}

//************ The set of comparison functors *******************
// functors are equivalent to corresponding std functors (e.g. std::less) but
// are not templated

/**
 * @brief greater functor
 */
struct greater {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param x
     * @param y
     *
     * @return
     */
    template <class T>
    auto operator()(const T &x, const T &y) const->decltype(x > y) {
        return x > y;
    }
};

/**
 * @brief less functor
 */
struct less {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param x
     * @param y
     *
     * @return
     */
    template <class T>
    auto operator()(const T &x, const T &y) const->decltype(x < y) {
        return x < y;
    }
};

/**
 * @brief greater_equal functor
 */
struct greater_equal {
    /**
     * @brief operator()
     *
     * @tparam T1
     * @tparam T2
     * @param x
     * @param y
     *
     * @return
     */
    template <class T1, class T2>
    auto operator()(const T1 &x, const T2 &y) const->decltype(x >= y) {
        return x >= y;
    }
};

/**
 * @brief less_equal functor
 */
struct less_equal {
    /**
     * @brief operator()
     *
     * @tparam T1
     * @tparam T2
     * @param x
     * @param y
     *
     * @return
     */
    template <class T1, class T2>
    auto operator()(const T1 &x, const T2 &y) const->decltype(x <= y) {
        return x <= y;
    }
};

/**
 * @brief equal_to functor
 */
struct equal_to {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param x
     * @param y
     *
     * @return
     */
    template <class T>
    auto operator()(const T &x, const T &y) const->decltype(x == y) {
        return x == y;
    }
};

/// TODO equivalent to c++14 equal_to<>, remove when appears
struct equal_to_unspecified {
    /**
     * @brief operator()
     *
     * @tparam T
     * @tparam U
     * @param t
     * @param u
     *
     * @return
     */
    template <class T, class U>
    auto operator()(T &&t, U &&u) const->decltype(t == u) {
        return t == u;
    }
};

/**
 * @brief not_equal_to functor
 */
struct not_equal_to {
    /**
     * @brief operator
     *
     * @tparam T
     * @param x
     * @param y
     *
     * @return
     */
    template <class T>
    auto operator()(const T &x, const T &y) const->decltype(x != y) {
        return x != y;
    }
};

/// This comparator  takes functor "f" and comparator "c"
/// and for elements(x,y) returns c(f(x), f(y))
/// c is less by default
template <typename Functor, typename Compare = less>
struct functor_to_comparator {
    /**
     * @brief constructor
     *
     * @param f
     * @param c
     */
    functor_to_comparator(Functor f, Compare c = Compare()) : m_f(f), m_c(c) {}

    /**
     * @brief operator()
     *
     * @tparam T
     * @param left
     * @param right
     *
     * @return
     */
    template <typename T>
    auto operator()(const T &left, const T &right) const->decltype(
        std::declval<Compare>()(std::declval<Functor>()(left),
                                std::declval<Functor>()(right))) {
        return m_c(m_f(left), m_f(right));
    }

  private:
    Functor m_f;
    Compare m_c;
};

/**
 * @brief make for functor to comparator
 *
 * @tparam Functor
 * @tparam Compare
 * @param functor
 * @param compare
 *
 * @return
 */
template <typename Functor, typename Compare = less>
auto make_functor_to_comparator(Functor functor, Compare compare = Compare()) {
    return functor_to_comparator<Functor, Compare>(std::move(functor),
                                                   std::move(compare));
}

/// Functor that scales another functor
template <typename Functor, typename ScaleType,
          typename ReturnType = ScaleType>
struct scale_functor {

    /**
     * @brief constructor
     *
     * @param f
     * @param s
     */
    scale_functor(Functor f, ScaleType s) : m_f(std::move(f)), m_s(s) {}

    /**
     * @brief operator()
     *
     * @tparam Arg
     * @param arg
     *
     * @return
     */
    template <typename Arg> ReturnType operator()(Arg &&arg) const {
        return m_s * m_f(std::forward<Arg>(arg));
    }

  private:
    Functor m_f;
    ScaleType m_s;
};

/**
 * @brief make for scale_functor
 *
 * @tparam ScaleType
 * @tparam ReturnType
 * @tparam Functor
 * @param f
 * @param s
 *
 * @return
 */
template <typename ScaleType, typename ReturnType = ScaleType,
          typename Functor>
auto make_scale_functor(Functor f, ScaleType s) {
    return scale_functor<Functor, ScaleType, ReturnType>(f, s);
}

//****************************** This is a set of functors representing standard
// arithmetic
// operations that is +, -, etc. These are equivalent to standard std:: structs
// but are not templated
/// plus
struct plus {
    /**
     * @brief operator()
     *
     * @tparam T1
     * @tparam T2
     * @param left
     * @param right
     */
    template <typename T1, typename T2>
    auto operator()(T1&& left, T2&& right) const
    -> decltype(std::forward<T1>(left) + std::forward<T2>(right)) {
        return std::forward<T1>(left) + std::forward<T2>(right);
    }
    // TODO change other operations (minus, max, less, etc..) like this one:
    // with two different types and perfect forwarding
};

/// minus
struct minus {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param left
     * @param right
     */
    template <typename T>
    auto operator()(const T &left,
                    const T &right) const->decltype(left - right) {
        return left - right;
    }
};

/// max
struct max {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param left
     * @param right
     */
    template <typename T>
    auto operator()(const T &left,
                    const T &right) const->decltype(std::max(left, right)) {
        return std::max(left, right);
    }
};

/// min
struct min {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param left
     * @param right
     */
    template <typename T>
    auto operator()(const T &left,
                    const T &right) const->decltype(std::min(left, right)) {
        return std::min(left, right);
    }
};

//****************************** This is a set of functors representing standard
// boolean operations
// that is !, &&, ||. These are equivalent to standard std:: structs but are not
// templated
//(only operator() is templated)

/// Not
struct Not {
    /**
     * @brief operator()
     *
     * @tparam T
     *
     * @return
     */
    template <typename T> auto operator()(const T &b) const->decltype(!b) {
        return !b;
    }
};

/// Or
struct Or {
    /**
     * @brief operator
     *
     * @tparam T
     * @param left
     * @param right
     *
     * @return
     */
    template <typename T>
    auto operator()(const T &left,
                    const T &right) const->decltype(left || right) {
        return left || right;
    }
};

/// And
struct And {
    /**
     * @brief operator()
     *
     * @tparam T
     * @param left
     * @param right
     *
     * @return
     */
    template <typename T>
    auto operator()(const T &left,
                    const T &right) const->decltype(left &&right) {
        return left && right;
    }
};

/// Functor stores binary operator "o" and two functors "f" and "g"
/// for given "args" returns o(f(args), g(args))
template <typename FunctorLeft, typename FunctorRight, typename Operator>
struct lift_binary_operator_functor {
    /**
     * @brief constructor
     *
     * @param left
     * @param right
     * @param op
     */
    lift_binary_operator_functor(FunctorLeft left = FunctorLeft(),
                                 FunctorRight right = FunctorRight(),
                                 Operator op = Operator())
        : m_left(std::move(left)), m_right(std::move(right)),
          m_operator(std::move(op)) {}

    /**
     * @brief operator
     *
     * @tparam Args
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args) const->decltype(std::declval<Operator>()(
        std::declval<FunctorLeft>()(std::forward<Args>(args)...),
        std::declval<FunctorRight>()(std::forward<Args>(args)...))) {
        return m_operator(m_left(std::forward<Args>(args)...),
                          m_right(std::forward<Args>(args)...));
    }

  private:
    FunctorLeft m_left;
    FunctorRight m_right;
    Operator m_operator;
};

/**
 * @brief make function for lift_binary_operator_functor
 *
 * @tparam FunctorLeft
 * @tparam FunctorRight
 * @tparam Operator
 * @param left
 * @param right
 * @param op
 *
 * @return
 */
template <typename FunctorLeft, typename FunctorRight, typename Operator>
auto make_lift_binary_operator_functor(FunctorLeft left, FunctorRight right,
                                       Operator op) {
    return lift_binary_operator_functor<FunctorLeft, FunctorRight, Operator>(
        std::move(left), std::move(right), std::move(op));
}

//******************** this is set of functors
// allowing to compose functors using
// standard logical operators

/// not_functor
template <typename Functor> struct not_functor {
    /**
     * @brief constructor
     *
     * @param functor
     */
    not_functor(Functor functor = Functor()) : m_functor(functor) {}

    /**
     * @brief operator()
     *
     * @tparam Args
     *
     * @return
     */
    template <typename... Args>
    auto operator()(Args &&... args) const->decltype(
        std::declval<Functor>()(std::forward<Args>(args)...)) {
        return !m_functor(std::forward<Args>(args)...);
    }

  private:
    Functor m_functor;
};

/// make for Not
template <typename Functor>
auto make_not_functor(Functor functor) {
    return not_functor<Functor>(std::move(functor));
}

/// or_functor
template <typename FunctorLeft, typename FunctorRight>
struct or_functor
    : public lift_binary_operator_functor<FunctorLeft, FunctorRight, Or> {
    typedef lift_binary_operator_functor<FunctorLeft, FunctorRight, Or> base;

    /**
     * @brief constructor
     *
     * @param left
     * @param right
     */
    or_functor(FunctorLeft left = FunctorLeft(),
               FunctorRight right = FunctorRight())
        : base(std::move(left), std::move(right)) {}
};

/// make for or_functor
template <typename FunctorLeft, typename FunctorRight>
auto make_or_functor(FunctorLeft left,
                     FunctorRight right) {
    return or_functor<FunctorLeft, FunctorRight>(std::move(left),
                                                 std::move(right));
}

/// and_functor
template <typename FunctorLeft, typename FunctorRight>
struct and_functor
    : public lift_binary_operator_functor<FunctorLeft, FunctorRight, And> {
    typedef lift_binary_operator_functor<FunctorLeft, FunctorRight, And> base;

    /**
     * @brief constructor
     *
     * @param left
     * @param right
     */
    and_functor(FunctorLeft left = FunctorLeft(),
                FunctorRight right = FunctorRight())
        : base(std::move(left), std::move(right)) {}
};

/// make and_functor
template <typename FunctorLeft, typename FunctorRight>
auto make_and_functor(FunctorLeft left, FunctorRight right) {
    return and_functor<FunctorLeft, FunctorRight>(std::move(left),
                                                  std::move(right));
}

/// xor_functor
template <typename FunctorLeft, typename FunctorRight>
struct xor_functor : public lift_binary_operator_functor<
    FunctorLeft, FunctorRight, not_equal_to> {
    typedef lift_binary_operator_functor<FunctorLeft, FunctorRight,
                                         not_equal_to> base;

    /**
     * @brief constructor
     *
     * @param left
     * @param right
     */
    xor_functor(FunctorLeft left = FunctorLeft(),
                FunctorRight right = FunctorRight())
        : base(std::move(left), std::move(right)) {}
};

/// make for Xor
template <typename FunctorLeft, typename FunctorRight>
auto make_xor_functor(FunctorLeft left, FunctorRight right) {
    return xor_functor<FunctorLeft, FunctorRight>(std::move(left),
                                                  std::move(right));
}

/// functor for std::tuple::get<I>
template <std::size_t I>
struct tuple_get {
    /**
     * @brief operator()
     *
     * @tparam Tuple
     * @return
     */
    template <typename Tuple>
    //TODO change to decltype(auto), when it starts working
    auto operator()(Tuple &&tuple) const ->
            decltype(std::get<I>(std::forward<Tuple>(tuple))) {
        return std::get<I>(std::forward<Tuple>(tuple));
    }
};

} //! utils
} //! paal
#endif // PAAL_FUNCTORS_HPP
