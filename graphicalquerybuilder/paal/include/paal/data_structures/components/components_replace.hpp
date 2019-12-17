//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file components_replace.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-24
 */
#ifndef PAAL_COMPONENTS_REPLACE_HPP
#define PAAL_COMPONENTS_REPLACE_HPP

#include "paal/data_structures/components/components.hpp"

namespace paal {
namespace data_structures {

/**
 * @brief Generic version of replaced_type
 *
 * @tparam Name
 * @tparam NewType
 * @tparam components
 */
template <typename Name, typename NewType, typename components>
class replaced_type;

/**
 * @class replaced_type
 * @brief Returns type of  components<Names, Types>, with Type for Name change
 * to NewType
 *
 * @tparam Name name of the changed type
 * @tparam NewType new type for Name
 * @tparam Names names list
 * @tparam Types old types list
 */
template <typename Name, typename NewType, typename Names, typename Types>
class replaced_type<Name, NewType, detail::components<Names, Types>> {
    static const int p = pos<Name, Names>::value; // position to insert
    typedef typename replace_at_pos<p, NewType, Types>::type TypesReplace;

  public:
    typedef detail::components<Names, TypesReplace> type;
};

namespace detail {

/**
 * @brief generic get_types
 *
 * @tparam Comp
 */
template <typename Comp> struct get_types;

/**
 * @class get_types
 * @brief gets types list for components class
 *
 * @tparam Names
 * @tparam Types
 */
template <typename Names, typename Types>
struct get_types<components<Names, Types>> {
    typedef Types type;
};

/**
 * @class TempReplacecomponents
 * @brief This class behavies like partial components<Names, Types>,
 *        with type for Name chanche to Type
 *
 * @tparam Name changed name
 * @tparam NewType new type
 * @tparam Names all names
 * @tparam Types aol types
 */
template <typename Name, typename NewType, typename Names, typename Types>
class temp_replaced_components {
    typedef detail::components<Names, Types> Comps;
    typedef typename replaced_type<Name, NewType, Comps>::type Replaced;
    typedef typename detail::get_types<Replaced>::type NewTypes;

  public:
    temp_replaced_components(const Comps &comps, const NewType &comp)
        : m_comps(comps), m_comp(comp) {}

    template <typename ComponentName>
    const typename detail::type_for_name<ComponentName, Names, NewTypes>::type &
    get() const {
        return get(detail::wrap_to_constructable<ComponentName>());
    }

  private:

    template <typename ComponentName>
    auto get(detail::wrap_to_constructable<ComponentName>) const->decltype(
        std::declval<const Comps>().template get<ComponentName>()) {
        return m_comps.template get<ComponentName>();
    }

    const NewType &get(detail::wrap_to_constructable<Name>) const {
        return m_comp;
    }

    const Comps &m_comps;
    const NewType &m_comp;
};
}

/**
 * @brief This function, for a specific Name, replaces compoonent in the
 * components class.
 *        The comonent should have deifferent type than prevoius component for
 * this Name
 *        (If the type is the same, set member function from components class
 * chould be used).
 *        The function returns components class fo type replaced_type<Name, NewType, Oldcomponents >::type.
 *        The function creates temporary object wich behaves like result
 * components
 *        and creates final object calling special Copy constructor.
 *
 * @tparam Name
 * @tparam NewType
 * @tparam Names
 * @tparam Types
 * @param comp
 * @param components
 *
 * @return
 */
template <typename Name, typename NewType, typename Names, typename Types>
typename replaced_type<Name, NewType, detail::components<Names, Types>>::type
replace(NewType comp, detail::components<Names, Types> components) {
    typedef detail::components<Names, Types> Comps;
    typedef typename replaced_type<Name, NewType, Comps>::type Replaced;

    return Replaced(
        detail::temp_replaced_components<Name, NewType, Names, Types>(
            components, comp),
        copy_tag());
}

} // data_structures
} // paal

#endif // PAAL_COMPONENTS_REPLACE_HPP
