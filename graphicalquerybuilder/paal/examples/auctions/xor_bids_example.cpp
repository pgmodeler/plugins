//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file xor_bids_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-6-8
 */

#include "paal/auctions/xor_bids.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

//! [Xor Bids Auctions Example]
int main()
{
   using Bidder = std::string;
   using Item = std::string;
   using Items = std::unordered_set<Item>;
   using Value = int;
   using Bid = std::pair<Items, Value>;
   using Bids = std::vector<Bid>;
   namespace pa = paal::auctions;

   std::unordered_map<Bidder, Bids> bids {
      {"Pooh Bear", {
         {{"honey"}, 10},
      }},
      {"Rabbit", {
         {{"baby carrot"}, 1},
         {{"carrot"}, 2},
         {{"baby carrot", "carrot"}, 4},
      }},
   };
   std::vector<Bidder> bidders {"Pooh Bear", "Rabbit"};
   std::vector<Item> items {"honey", "baby carrot", "carrot", "jam"};
   auto get_bids = [&](Bidder bidder) -> const Bids& { return bids.at(bidder); };
   auto get_value = [](const Bid& bid) { return bid.second; };
   auto get_items = [](const Bid& bid) -> const Items& { return bid.first; };
   auto get_copies_num = [](Item item) { return item == "baby carrot" ? 2 : 1; };

   auto auction = pa::make_xor_bids_to_value_query_auction(
      bidders, items, get_bids, get_value, get_items, get_copies_num
   );

   Items bundle {"carrot", "honey", "baby carrot"};

   std::cout << "pooh bear valuation: " <<
      auction.call<pa::value_query>("Pooh Bear", bundle) << std::endl;

   std::cout << "rabbit valuation: " <<
      auction.call<pa::value_query>("Rabbit", bundle) << std::endl;

   return 0;
}
//! [Xor Bids Auctions Example]
