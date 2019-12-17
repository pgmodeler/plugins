//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file value_query_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-6-5
 */

#include "paal/auctions/auction_components.hpp"

#include <boost/range/algorithm/copy.hpp>

#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include <set>
#include <unordered_set>
#include <vector>

//! [Value Query Auction Components Example]

namespace pa = paal::auctions;

using Bidder = std::string;
using Item = std::string;
using Value = unsigned int;

const std::vector<Bidder> bidders {"Pooh Bear", "Rabbit"};

const std::vector<Item> items {"honey", "baby carrot", "carrot", "jam"};

struct get_copies_num_func {
   int operator()(Item item) const
   {
      return item == "baby carrot" ? 2 : 1;
   }
};

struct value_query_func {
   template <class ItemSet>
   Value operator()(Bidder bidder, const ItemSet& item_set) const
   {
      if (bidder == "Pooh Bear")
         return item_set.count("honey") > 0 ? 10 : 0;
      assert(bidder == "Rabbit");
      Value res = 0;
      for (Item item: item_set)
         if (item.find("carrot") != std::string::npos) ++res;
      return res;
   }
};

//! [Value Query Auction Components Example]

int main()
{
   //! [Value Query Auction Create Example]
   auto const auction = pa::make_value_query_auction_components(
      bidders, items, value_query_func(), get_copies_num_func()
   );
   //! [Value Query Auction Create Example]

   //! [Value Query Auction Use Example]
   std::cout << "bidders: ";
   boost::copy(
      auction.get<pa::bidders>(), std::ostream_iterator<Bidder>(std::cout, ", ")
   );
   std::cout << std::endl;

   std::cout << "items with copies numbers: ";
   for (auto item: auction.get<pa::items>())
      std::cout << item << " = " << auction.call<pa::get_copies_num>(item) << ", ";
   std::cout << std::endl;

   std::cout << "pooh bear valuation: " <<
      auction.call<pa::value_query>("Pooh Bear", std::set<Item>{"jam", "honey"}) <<
      std::endl;

   std::cout << "rabbit valuation: " <<
      auction.call<pa::value_query>(
         "Rabbit", std::unordered_set<Item>{"carrot", "baby carrot"}) <<
      std::endl;
   //! [Value Query Auction Use Example]

   return 0;
}
