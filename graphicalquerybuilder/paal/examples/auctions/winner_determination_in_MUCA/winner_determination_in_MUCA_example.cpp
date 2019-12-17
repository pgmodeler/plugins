//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file winner_determination_in_MUCA_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-1-9
 */

#include "paal/auctions/auction_components.hpp"
#include "paal/auctions/xor_bids.hpp"
#include "paal/auctions/winner_determination_in_MUCA/winner_determination_in_MUCA.hpp"

#include <boost/function_output_iterator.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

//! [Winner Determination In MUCA Example]

int main()
{
   using Bidder = std::string;
   using Item = std::string;
   using Items = std::unordered_set<Item>;
   using Value = double;
   using Bid = std::pair<Items, Value>;
   using Bids = std::vector<Bid>;

   // create auction
   const std::unordered_map<Bidder, Bids> bids {
      {"John", {
         {{"ball", "kite"}, 2},
         {{"umbrella"}, 3},
         {{"orange"}, 1.75},
         {{"ball", "kite", "umbrella"}, 5},
         {{"ball", "kite", "orange", "umbrella"}, 6.75},
      }},
      {"Bob", {
         {{"orange"}, 1.5},
         {{"apple"}, 2.0},
         {{"apple", "orange"}, 4},
      }},
      {"Steve", {
         {{"apple"}, 1},
         {{"umbrella"}, 4},
         {{"apple", "umbrella"}, 5},
      }},
   };
   const std::vector<Bidder> bidders {"John", "Bob", "Steve"};
   const std::vector<Item> items {"apple", "ball", "orange", "kite", "umbrella"};
   auto get_bids = [&](const Bidder& bidder) -> const Bids& { return bids.at(bidder); };
   auto get_value = [](const Bid& bid) { return bid.second; };
   auto get_items = [](const Bid& bid) -> const Items& { return bid.first; };
   auto auction = paal::auctions::make_xor_bids_to_gamma_oracle_auction(
      bidders, items, get_bids, get_value, get_items
   );

   // determine winners
   Value social_welfare = 0;
   auto valuation = paal::auctions::make_xor_bids_to_value_query_auction(
      bidders, items, get_bids, get_value, get_items
   );
   paal::auctions::determine_winners_in_gamma_oracle_auction(
      auction,
      boost::make_function_output_iterator([&](std::pair<Bidder, Items> p)
      {
         auto bidder = p.first;
         auto& cur_items = p.second;
         social_welfare += valuation.call<paal::auctions::value_query>(bidder, cur_items);
         std::cout << bidder << " got bundle: ";
         boost::copy(cur_items, std::ostream_iterator<Item>(std::cout, ", "));
         std::cout << std::endl;
      })
   );
   std::cout << "social welfare: " << social_welfare << std::endl;

   return 0;
}

//! [Winner Determination In MUCA Example]
