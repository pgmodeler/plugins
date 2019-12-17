//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file components.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-16
 */
#ifndef PAAL_COMPONENTS_HPP
#define PAAL_COMPONENTS_HPP

#include "paal/data_structures/components/types_vector.hpp"

#include <utility>
#include <iostream>

namespace paal {
namespace data_structures {

/**
 * @brief This structure can be passed on Names list and represents Name and the
 * default type value
 *
 * @tparam Name
 * @tparam Default
 */
template <typename Name, typename Default> struct NameWithDefault;

/**
 * @brief Indicates that components constructor is in fact a Copy/Move
 * Constructor
 */
struct copy_tag {};

// This namespace block contains implementation of the main class
// components<Names,Types> and needed meta functions
namespace detail {

///wraps type to constructible type
template <typename T> struct wrap_to_constructable {
    typedef T type;
};


///If Name is kth on Names list, returns kth Type.
template <typename Name, typename Names, typename Types> struct type_for_name {
    typedef typename remove_n_first<1, Names>::type NewNames;
    typedef typename remove_n_first<1, Types>::type NewTypes;
    typedef typename type_for_name<Name, NewNames, NewTypes>::type type;
};

///Specialization when found
template <typename Name, typename Type, typename... NamesRest,
          typename... TypesRest>
struct type_for_name<Name, TypesVector<Name, NamesRest...>,
                     TypesVector<Type, TypesRest...>> {
    typedef Type type;
};


///SFINAE check if the given type has get<Name>() member function.
template <typename T, typename Name> class has_template_get {
  private:
    /**
     * @brief positive case
     *
     * @tparam C given type
     *
     * @return return type is char
     */
    template <typename C>
    static char f(wrap_to_constructable<decltype(
        std::declval<const C>().template get<Name>())(C::*)() const> *);

    /**
     * @brief negative case
     *
     * @tparam C given type
     *
     * @return return type is long
     */
    template <typename C> static long f(...);

  public:
    /**
     * @brief tels if given type has get<Name>() memer function.
     *
     */
    static const bool value =
        (sizeof(f<typename std::decay<T>::type>(nullptr)) == sizeof(char));
};

/**
 * @brief Tag indicating that given object is movable
 */
struct movable_tag {};
/**
 * @brief Tag indicating that given object is not movable
 */
struct Notmovable_tag {};

// declaration of main class components
template <typename Names, typename Types> class components;

// specialization for empty Names list
template <> class components<TypesVector<>, TypesVector<>> {
  public:
    void get() const;
    void call() const;
    void call2() const;

    template <typename... Unused> components(const Unused &...) {}
};

// specialization for nonempty types list
// class keeps first component as data memer
// rest of the components are kept in superclass.
template <typename Name, typename Type, typename... NamesRest,
          typename... TypesRest>
class components<TypesVector<Name, NamesRest...>,
                 TypesVector<Type, TypesRest...>> : public components<
    TypesVector<NamesRest...>, TypesVector<TypesRest...>> {
    typedef components<TypesVector<NamesRest...>, TypesVector<TypesRest...>>
        base;
    typedef TypesVector<Name, NamesRest...> Names;
    typedef TypesVector<Type, TypesRest...> Types;

    ///Evaluates to valid type iff componentsName == Name
    template <typename ComponentName>
    using is_my_name =
        typename std::enable_if<std::is_same<ComponentName, Name>::value>::type;

  public:
    using base::get;

    /// constructor
    // we do not use  = default, cause we'd like to value initialize POD's.
    components() : base{}, m_component{} {};

    //  copy constructor
    components(components const & other)
        : base(static_cast<base const &>(other)), m_component(other.get<Name>()) {}

    // doesn't work on clang 3.2 // change in the standard and visual studio 2015 preview
    // components(components &) = default;
    // constructor taking nonconst lvalue reference
    components(components &other)
        : base(static_cast<base &>(other)), m_component(other.get<Name>()) {}

    //  move constructor
    components(components &&) = default;

    // assignment operator
    components &operator=(components const & other) {

        static_cast<base &>(*this) = static_cast<base &>(other);
        m_component = other.get<Name>();
        return *this;
    }

    // doesn't work on clang 3.2 // change in the standard
    // components& operator=(components &) = default;
    // assignment operator taking nonconst lvalue reference
    components &operator=(components &other) {
        static_cast<base &>(*this) = static_cast<base &>(other);
        m_component = other.get<Name>();
        return *this;
    }

    // default move operator
    components &operator=(components &&) = default;

    /**
     * @brief constructor takes some number of arguments,
     *      This arguments has to be convertible to the same number of the first
     * components in components class.
     *      Arguments can be both rvalue and lvalue references
     *
     * @tparam T, first component, it must be convertible to Type.
     * @tparam TypesPrefix, rest of the components
     * @param t
     * @param types
     */
    template <typename T, typename... TypesPrefix>
    components(T &&t, TypesPrefix &&... types)
        : base(std::forward<TypesPrefix>(types)...),
          m_component(std::forward<T>(t)) {}

    //     copy constructor takes class wich has get<Name> member function
    //     the get<> function dosn't have to be available for all names.
    //     @param copy_tag is helps identify  this constructor
    template <typename Comps>
    components(const Comps &comps, copy_tag)
        : components(comps, Notmovable_tag()) {}

    //   move  constructor takes class wich has get<Name> member function
    //     the get<> function dosn't have to be available for all names.
    //     In this version each of the components taken from comps
    //     is going to be moved.
    //     @param copy_tag is helps identify  this constructor
    template <typename Comps>
    components(Comps &&comps, copy_tag)
        : components(comps, movable_tag()) {}

    /**
     * @brief This fucntion returns Component for name Name, nonconst version
     *
     * @tparam ComponentName
     * @tparam typename
     * @param dummy
     *
     * @return
     */
    template <typename ComponentName, typename = is_my_name<ComponentName>>
    Type &get(wrap_to_constructable<Name> dummy =
                  wrap_to_constructable<Name>()) {
        return m_component;
    }

    /**
     * @brief This fucntion returns Component for name Name, const version
     *
     * @tparam ComponentName
     * @tparam typename
     * @param dummy
     *
     * @return
     */
    template <typename ComponentName, typename = is_my_name<ComponentName>>
    const Type &get(wrap_to_constructable<Name> dummy =
                        wrap_to_constructable<Name>()) const {
        return m_component;
    }

    /**
     * @brief This function directly calls component.
     *        m_component(args) has to be valid expresion
     *        nonconst version
     *
     * @tparam ComponentName
     * @tparam Args
     * @param args call arguments
     *
     * @return
     */
    template <typename ComponentName, typename... Args>
    auto call(Args &&... args)->decltype(std::declval<
        typename type_for_name<ComponentName, Names, Types>::type>()(
        std::forward<Args>(args)...)) {
        return this->template get<ComponentName>()(std::forward<Args>(args)...);
    }

    /**
     * @brief This function directly calls component.
     *        m_component(args) has to be valid expresion
     *        const version
     *
     * @tparam ComponentName
     * @tparam ComponentName
     * @tparam Args
     * @param args call arguments
     *
     * @return the same as m_component return type
     */
    template <typename ComponentName, typename... Args>
    auto call(Args &&... args) const->decltype(std::declval<
        const typename type_for_name<ComponentName, Names, Types>::type>()(
        std::forward<Args>(args)...)) {
        return this->template get<ComponentName>()(std::forward<Args>(args)...);
    }

    /**
     * @brief setter for component assigned to Name.
     *
     * @tparam ComponentName
     * @param comp
     */
    template <typename ComponentName>
    void
    set(const typename type_for_name<ComponentName, Names, Types>::type comp) {
        this->get<ComponentName>() = std::move(comp);
    }

    /**
     * @brief function creating components class,
     *        takes arguments only for assigned Names
     *
     * @tparam NamesSubset
     * @tparam SomeTypes
     * @param types
     *
     * @return
     */
    template <typename... NamesSubset, typename... SomeTypes>
    static components<Names, Types>
        // make(SomeTypes... types) {
        //  static_assert(sizeof...(NamesSubset) == sizeof...(SomeTypes),
        // "Incorrect number of arguments.");
        // return components<Names,
        // Types>(components<TypesVector<NamesSubset...>,
        // TypesVector<SomeTypes...>>(std::move(types)...), copy_tag());
        make(SomeTypes &&... types) {
        static_assert(sizeof...(NamesSubset) == sizeof...(SomeTypes),
                      "Incorrect number of arguments.");
        components<TypesVector<NamesSubset...>, TypesVector<SomeTypes...>>
            comps(std::forward<SomeTypes>(types)...);
        return components<Names, Types>(std::move(comps), copy_tag());
    }

  protected:

    // object is moved if move = true, otherwise passed by reference
    template <bool move, typename A> A move_or_pass_reference(const A &a) {
        return std::move(a);
    }

    // const reference case
    template <bool move, typename A,
              typename = typename std::enable_if<!move>::type>
    const A &move_or_pass_reference(const A &a) {
        return a;
    }

    // nonconst reference case
    template <bool move, typename A,
              typename = typename std::enable_if<!move>::type>
    A &move_or_pass_reference(A &a) {
        return a;
    }

    // All of this constructor takes Comps as r-value reference,
    // because they have to win specialization race with normal constructor.

    // case: movable object, has the appropriate get member function
    template <typename Comps,
              typename dummy = typename std::enable_if<
                  has_template_get<Comps, Name>::value, int>::type>
    components(Comps &&comps, movable_tag m, dummy d = dummy())
        : base(std::forward<Comps>(comps), std::move(m)),
          // if Type is not reference type, comps.get<Name>() is moved otherwise
          // reference is  passed
          m_component(
              move_or_pass_reference<!std::is_lvalue_reference<Type>::value>(
                  comps.template get<Name>())) {}

    // case: movable object, does not have the appropriate get member function
    template <typename Comps, typename dummy = typename std::enable_if<
                                  !has_template_get<Comps, Name>::value>::type>
    components(Comps &&comps, movable_tag m)
        : base(std::forward<Comps>(comps), std::move(m)) {}

    // case: not movable object, has the appropriate get member function
    template <typename Comps,
              typename dummy = typename std::enable_if<
                  has_template_get<Comps, Name>::value, int>::type>
    components(Comps &&comps, Notmovable_tag m, dummy d = dummy())
        : base(std::forward<Comps>(comps), std::move(m)),
          m_component(comps.template get<Name>()) {}

    // case: not movable object, does not  have the appropriate get member
    // function
    template <typename Comps, typename dummy = typename std::enable_if<
                                  !has_template_get<Comps, Name>::value>::type>
    components(Comps &&comps, Notmovable_tag m)
        : base(std::forward<Comps>(comps), std::move(m)) {}

  private:
    Type m_component;
};
} // detail

//This namespace contains class which sets all defaults and all needed meta functions.

namespace detail {

template <typename Names, typename Defaults, typename TypesPrefix>
class set_defaults {
    static const int N = size<Names>::value;
    static const int TYPES_NR = size<TypesPrefix>::value;
    static_assert(TYPES_NR <= N, "Incrrect number of parameters");

    static const int DEFAULTS_NR = size<Defaults>::value;
    static_assert(DEFAULTS_NR + TYPES_NR >= N, "Incrrect number of parameters");

    typedef typename remove_n_first<DEFAULTS_NR + TYPES_NR - N, Defaults>::type
        NeededDefaults;

    typedef typename join<TypesPrefix, NeededDefaults>::type Types;

  public:
    typedef detail::components<Names, Types> type;
};
} // detail

//Here are some meta functions, to parse the arguments
namespace detail {
/**
 * @brief get_name, gets name for either Name, or NamesWithDefaults struct
 *        this is the Name case
 *
 * @tparam T
 */
template <typename T> struct get_name {
    typedef T type;
};

/**
 * @brief get_name, gets name for either Name, or NamesWithDefaults struct
 *        this is the NamesWithDefaults case
 *
 * @tparam Name
 * @tparam Default
 */
template <typename Name, typename Default>
struct get_name<NameWithDefault<Name, Default>> {
    typedef Name type;
};

/**
 * @brief Meta function takes NameWithDefault and Vector
 *        the result is new vector with new Name appended Name
 */
struct push_back_name {
    template <typename Vector, typename NameWithDefault> struct apply {
        typedef typename push_back<
            Vector, typename get_name<NameWithDefault>::type>::type type;
    };
};

/*
 * @brief Meta function takes NameWithDefault and Vector
 *        the result is new vector with new Name appended Default
 */
struct push_back_default {
    //  This case applies to when NameWithDefault is only name
    template <typename Vector, typename Name> struct apply {
        typedef Vector type;
    };

    // This case applies when NameWithDefault contains Default
    template <typename Vector, typename Name, typename Default>
    struct apply<Vector, NameWithDefault<Name, Default>> {
        typedef typename push_back<Vector, Default>::type type;
    };
};
} // detail

/// this is class sets all defaults and return as type detail::components<Names,
/// Types>
/// direct implementation on variadic templates is imposible because of
/// weak support for type detection for inner template classes
template <typename... ComponentNamesWithDefaults> class components {
    typedef TypesVector<ComponentNamesWithDefaults...> NamesWithDefaults;

    /// get Names list from NamesWithDefaults
    typedef typename fold<NamesWithDefaults, TypesVector<>,
                          detail::push_back_name>::type Names;

    /// get Defaults from NamesWithDefaults
    typedef typename fold<NamesWithDefaults, TypesVector<>,
                          detail::push_back_default>::type Defaults;

    /**
     * @brief for detecting references adapters
     *
     * @tparam T
     */
    template <class T> struct special_decay {
        using type = typename std::decay<T>::type;
    };

    /**
     * @brief specialization, when type is surrounded by std::ref
     *
     * @tparam T
     */
    template <class T> struct special_decay<std::reference_wrapper<T>> {
        using type = T &;
    };

    template <class T> using special_decay_t = typename special_decay<T>::type;

  public:
    template <typename... ComponentTypes>
    using type = typename detail::set_defaults<
        Names, Defaults, TypesVector<ComponentTypes...>>::type;

    /// make function for components
    template <typename... components>
    static type<special_decay_t<components>...>
    make_components(components &&... comps) {
        return type<special_decay_t<components>...>(
            std::forward<components>(comps)...);
    }

  private:
    // in this block we check if the defaults are on the last positions in the
    // NamesWithDefaults
    static const int N = size<NamesWithDefaults>::value;
    static const int DEFAULTS_NR = size<Defaults>::value;
    typedef typename remove_n_first<N - DEFAULTS_NR, NamesWithDefaults>::type
        DefaultPart;
    typedef typename fold<DefaultPart, TypesVector<>,
                          detail::push_back_default>::type DefaultsTest;
    static_assert(std::is_same<DefaultsTest, Defaults>::value,
                  "Defaults values could be only on subsequent number of last "
                  "parameters");
};

} //! data_structures
} //! paal
#endif // PAAL_COMPONENTS_HPP
