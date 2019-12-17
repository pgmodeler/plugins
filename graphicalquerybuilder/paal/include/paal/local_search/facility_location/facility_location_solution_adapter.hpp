//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file facility_location_solution_adapter.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-01
 */

#ifndef PAAL_FACILITY_LOCATION_SOLUTION_ADAPTER_HPP
#define PAAL_FACILITY_LOCATION_SOLUTION_ADAPTER_HPP

#define BOOST_RESULT_OF_USE_DECLTYPE

#include "paal/utils/type_functions.hpp"
#include "paal/utils/functors.hpp"
#include "paal/data_structures/collection_starts_from_last_change.hpp"
#include "paal/data_structures/facility_location/facility_location_solution_traits.hpp"

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/distance.hpp>
#include <boost/range/algorithm/find.hpp>

#include <unordered_map>
#include <unordered_set>

namespace paal {
namespace local_search {

/**
 * @brief facility_location_solution adapter
 *          chosen range and unchosen range must be joined into one homogenus
* collection of Facilities.
 *
 * @tparam facility_location_solution
 */
template <typename facility_location_solution>
class facility_location_solution_adapter {
    typedef facility_location_solution FLS;

    //TODO in fractioned MUCA commit, there will be function separated for that
    template <typename Collection, typename Range
                = const typename boost::iterator_range<typename Collection::const_iterator>::type>
    auto get_cycledCopy(const Collection &col, std::size_t index) const
            -> boost::joined_range<Range, Range>  {
        return boost::join(
            boost::make_iterator_range(col.begin() + index, col.end()),
            boost::make_iterator_range(col.begin(), col.begin() + index));
    }

  public:
    typedef typename facility_location_solution::VertexType VertexType;
    /// type of Chosen collection
    typedef decltype(std::declval<FLS>().get_chosen_facilities()) Chosen;
    /// type of Unchosen collection
    typedef decltype(std::declval<FLS>().get_unchosen_facilities()) Unchosen;
    typedef typename data_structures::facility_location_solution_traits<
        FLS>::Dist Dist;
    typedef std::vector<VertexType> UnchosenCopy;
    typedef std::vector<VertexType> ChosenCopy;

  private:
    facility_location_solution &m_sol;
    /// copy of all unchosen facilities
    UnchosenCopy m_unchosen_copy;
    /// copy of all chosen facilities
    ChosenCopy m_chosen_copy;
    /// index of last facility removed from unchosen
    std::size_t m_last_used_unchosen;
    /// index of last facility removed from chosen
    std::size_t m_last_used_chosen;

public:
    /**
     * @brief constructor creates cycled range of all facilities
     *
     * @param sol
     */
    facility_location_solution_adapter(facility_location_solution &sol)
        : m_sol(sol), m_unchosen_copy(m_sol.get_unchosen_facilities().begin(),
                                      m_sol.get_unchosen_facilities().end()),
          m_chosen_copy(m_sol.get_chosen_facilities().begin(),
                        m_sol.get_chosen_facilities().end()),
          m_last_used_unchosen{}, m_last_used_chosen{} {}

    /**
     * @brief adds facility tentatively (used in gain computation).
     *
     * @param v
     *
     * @return
     */
    Dist add_facility_tentative(VertexType v) { return m_sol.add_facility(v); }

    /**
     * @brief adds facility
     *
     * @param v
     *
     * @return
     */
    Dist add_facility(VertexType v) {
        auto ret = add_facility_tentative(v);
        auto elemIter = boost::range::find(m_unchosen_copy, v);
        assert(elemIter != m_unchosen_copy.end());
        elemIter = m_unchosen_copy.erase(elemIter);
        m_last_used_unchosen = elemIter - m_unchosen_copy.begin();
        m_chosen_copy.push_back(v);
        return ret;
    }

    /**
     * @brief ads facility tentatively (used in gain computation)
     *
     * @param v
     *
     * @return
     */
    Dist remove_facility_tentative(VertexType v) {
        return m_sol.rem_facility(v);
    }

    /**
     * @brief removes facility
     *
     * @param v
     *
     * @return
     */
    Dist remove_facility(VertexType v) {
        auto ret = remove_facility_tentative(v);
        m_unchosen_copy.push_back(v);
        auto elemIter = boost::range::find(m_chosen_copy, v);
        assert(elemIter != m_chosen_copy.end());
        elemIter = m_chosen_copy.erase(elemIter);
        m_last_used_chosen = elemIter - m_chosen_copy.begin();
        return ret;
    }

    /**
     * @brief get solution
     *
     * @return
     */
    facility_location_solution &getfacility_location_solution() {
        return m_sol;
    }

    /**
     * @brief gets solution
     *
     * @return
     */
    const facility_location_solution &getfacility_location_solution() const {
        return m_sol;
    }

    /**
     * @brief returns copy of unchosen facilities
     *
     * @return
     */
    auto getUnchosenCopy() const -> decltype(this->get_cycledCopy(
         m_unchosen_copy, m_last_used_unchosen)) {
        return get_cycledCopy(m_unchosen_copy, m_last_used_unchosen);
    }

    /**
     * @brief
     *
     * @brief returns copy of chosen facilities
     *
     * @return
     */
    auto getChosenCopy() const -> decltype(this->get_cycledCopy(
                m_chosen_copy, m_last_used_chosen)){
        return get_cycledCopy(m_chosen_copy, m_last_used_chosen);
    }
};

} // local_search
} // paal

#endif // PAAL_FACILITY_LOCATION_SOLUTION_ADAPTER_HPP
