//=======================================================================
// Copyright (c) 2013 Robert Rosolek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file fractional_winner_determination_in_MUCA_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-06-09
 */

#include "paal/auctions/auction_components.hpp"
#include "paal/auctions/fractional_winner_determination_in_MUCA/fractional_winner_determination_in_MUCA.hpp"
#include "paal/auctions/xor_bids.hpp"

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

//! [Fractional Winner Determination in MUCA Example]
int main()
{
   using Bidder = std::string;
   using Item = std::string;
   using Items = std::unordered_set<Item>;
   using Value = long double;
   using Bid = std::pair<Items, Value>;
   using Bids = std::vector<Bid>;
   using Assignment = std::tuple<Bidder, Items, double>;

   // create auction
   const std::unordered_map<Bidder, Bids> bids {
      {"John", {
         {{"lemon", "orange"}, 1},
         {{"apple", "ball"}, 1},
      }},
      {"Bob", {
         {{"lemon", "apple"}, 1},
         {{"orange", "ball"}, 1},
      }},
   };
   const std::vector<Bidder> bidders {"John", "Bob"};
   const std::vector<Item> items {"apple", "ball", "orange", "lemon"};
   auto get_bids = [&](const Bidder& bidder) -> const Bids& { return bids.at(bidder); };
   auto get_value = [](const Bid& bid) { return bid.second; };
   auto get_items = [](const Bid& bid) -> const Items& { return bid.first; };
   auto auction = paal::auctions::make_xor_bids_to_demand_query_auction(
      bidders, items, get_bids, get_value, get_items
   );

   // determine winners
   Value social_welfare = 0;
   auto valuation = paal::auctions::make_xor_bids_to_value_query_auction(
      std::move(bidders), std::move(items), get_bids, get_value, get_items
   );
   paal::auctions::fractional_determine_winners_in_demand_query_auction(
      auction,
      boost::make_function_output_iterator([&](Assignment a)
      {
         auto bidder = std::get<0>(a);
         auto& cur_items = std::get<1>(a);
         auto fraction = std::get<2>(a);
         social_welfare += fraction * valuation.call<paal::auctions::value_query>(bidder, cur_items);
         std::cout << bidder << " got a fraction " << fraction << " of bundle: ";
         boost::copy(cur_items, std::ostream_iterator<Item>(std::cout, ", "));
         std::cout << std::endl;
      })
   );
   std::cout << "social welfare: " << social_welfare << std::endl;

   return 0;
}
//! [Fractional Winner Determination in MUCA Example]
