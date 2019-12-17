//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file auction_utils.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-4-10
 */
#ifndef PAAL_AUCTION_UTILS_HPP
#define PAAL_AUCTION_UTILS_HPP

#include "paal/auctions/auction_components.hpp"
#include "paal/auctions/auction_traits.hpp"
#include "paal/utils/accumulate_functors.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/distance.hpp>
#include <boost/range/empty.hpp>

#include <utility>

namespace paal {
namespace auctions {

/**
 * @brief Returns the number of different kinds of items in an auction.
 *
 * @param auction
 * @tparam Auction
 */
template <class Auction>
auto items_number(Auction&& auction) {
   return boost::distance(auction.template get<items>());
}

/**
 * @brief Returns the number of bidders in an auction.
 *
 * @param auction
 * @tparam Auction
 */
template <class Auction>
auto bidders_number(Auction&& auction) {
   return boost::distance(auction.template get<bidders>());
}

/**
 * @brief Returns minimum number of copies of an item in an auction.
 *
 * @param auction
 * @tparam Auction
 */
template <class Auction>
typename paal::auctions::auction_traits<Auction>::copies_num_t
get_minimum_copies_num(Auction&& auction)
{
   assert(!boost::empty(auction.template get<items>()));
   using item = typename auction_traits<Auction>::item_t;
   auto get_copies_num_func = [&](item i)
   {
      return auction.template call<get_copies_num>(std::forward<item>(i));
   };
   return *min_element_functor(auction.template get<items>(), get_copies_num_func);
}

} //!auctions
} //!paal
#endif // PAAL_AUCTION_UTILS_HPP
