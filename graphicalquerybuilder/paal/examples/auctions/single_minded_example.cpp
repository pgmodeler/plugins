//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file single_minded_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-6-6
 */

#include "paal/auctions/single_minded_auctions.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

//! [Single Minded Auctions Example]
int main()
{
   using Bidder = std::string;
   using Item = std::string;
   using Items = std::unordered_set<Item>;
   using Value = int;
   using Bid = std::pair<Items, Value>;
   namespace pa = paal::auctions;

   std::unordered_map<Bidder, Bid> bids {
      {"Pooh Bear", {{"honey"}, 10}},
      {"Rabbit", {{"baby carrot", "carrot"}, 2}},
   };
   std::vector<Bidder> bidders {"Pooh Bear", "Rabbit"};
   std::vector<Item> items {"honey", "baby carrot", "carrot", "jam"};
   auto get_value = [&](Bidder bidder) { return bids.at(bidder).second; };
   auto get_items = [&](Bidder bidder) -> const Items& { return bids.at(bidder).first; };
   auto get_copies_num = [](Item item) { return item == "baby carrot" ? 2 : 1; };

   auto auction = pa::make_single_minded_to_value_query_auction(
      bidders, items, get_value, get_items, get_copies_num
   );

   Items bundle {"carrot", "honey", "jam"};

   std::cout << "pooh bear valuation: " <<
      auction.call<pa::value_query>("Pooh Bear", bundle) << std::endl;

   std::cout << "rabbit valuation: " <<
      auction.call<pa::value_query>("Rabbit", bundle) << std::endl;

   return 0;
}
//! [Single Minded Auctions Example]
