//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file object_with_copy.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_OBJECT_WITH_COPY_HPP
#define PAAL_OBJECT_WITH_COPY_HPP

namespace paal {
namespace data_structures {

/**
 * @class object_with_copy
 * @brief keeps object and its copy. Invoke all the member functions on both:
 * object and its copy.
 *        If you want to invoke member function on both objects, you run the
 * object_with_copy::invoke.
 *        If you want to run member function only on the copy you run
 * object_with_copy::invoke_on_copy.
 *
 * @tparam T type of the contain object
 */
template <typename T> class object_with_copy {
  public:
    typedef T ObjectType;

    /**
     * @brief constructor
     *
     * @param t
     */
    object_with_copy(T t) : m_obj(std::move(t)), m_copy(m_obj) {}

    /**
     * @brief invokes member function on object and copy
     *
     * @param f - pointer to member function of T
     * @param args - arguments for f
     *
     * @tparam F type of pointer to  member function
     * @tparam Args... types of member function arguments
     *
     * @return the same as f
     */
    // if you use *. in decltype instead of -> you get
    // "sorry, unimplemented: mangling dotstar_expr" :)
    template <typename F, typename... Args>
    typename std::result_of<F(T*, Args...)>::type
        invoke(F f, Args... args) {
        (m_copy.*(f))(args...);
        return (m_obj.*(f))(args...);
    }

    /**
     * @brief invokes member function on copy
     *
     * @param f - pointer to member function of T
     * @param args - arguments for f
     *
     * @tparam F type of pointer to  member function
     * @tparam Args... types of member function arguments
     *
     * @return the same as f
     */
    template <typename F, typename... Args>
    typename std::result_of<F(T*, Args...)>::type
    invoke_on_copy(F f, Args... args) const {
        return (m_copy.*(f))(args...);
    }

    /**
     * @brief easier way for invoking const member functions
     *
     * @return T*
     */
    const T *operator->() const { return &m_obj; }

    /**
     * @brief getter for inner object
     *
     * @return member object
     */
    T &get_obj() { return m_obj; }

    /**
     * @brief getter for inner object
     *
     * @return member object
     */
    const T &get_obj() const { return m_obj; }

  private:
    /**
     * @brief Object
     */
    T m_obj;
    /**
     * @brief Copy of the object
     */
    mutable T m_copy;
};

} // data_structures
} // paal

#endif // PAAL_OBJECT_WITH_COPY_HPP
