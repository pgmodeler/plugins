//=======================================================================
// Copyright (c) 2013 Robert Rosolek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file demand_query_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-6-9
 */

#include "paal/auctions/auction_components.hpp"

#include <boost/optional/optional.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <algorithm>
#include <cassert>
#include <vector>

//! [Demand Query Auction Components Example]

namespace pa = paal::auctions;
namespace pds = paal::data_structures;

using Bidder = std::string;
using Item = std::string;
using Items = std::vector<Item>;
using Value = int;

const std::vector<Bidder> bidders {"Pooh Bear", "Rabbit"};

const Items items {"honey", "baby carrot", "carrot", "jam"};

struct demand_query_func {
   template <class GetPrice>
   std::pair<Items, Value>
   operator()(Bidder bidder, GetPrice get_price) const
   {
      if (bidder == "Pooh Bear") {
         const Value util = 10 - get_price("honey");
         if (util <= 0) return std::make_pair(Items{}, 0);
         return std::make_pair(Items{"honey"}, util);
      }

      assert(bidder == "Rabbit");

      const Value baby_val = 2, val = 3;
      auto const baby_price = get_price("baby carrot"), price = get_price("carrot");

      const Value baby_util = baby_val - baby_price,
         util = val - price, both_util = baby_val + val - baby_price - price;

      if (baby_util <= 0 && util <= 0 && both_util <= 0) return std::make_pair(Items{}, 0);

      if (baby_util >= util && baby_util >= both_util)
         return std::make_pair(Items{"baby carrot"}, baby_util);

      if (util >= both_util)
         return std::make_pair(Items{"carrot"}, util);

      return std::make_pair(Items{"baby carrot", "carrot"}, both_util);
   }
};
//! [Demand Query Auction Components Example]

int main() {
   //! [Demand Query Auction Create Example]
   auto const auction = pa::make_demand_query_auction_components(
      bidders, items, demand_query_func()
   );
   //! [Demand Query Auction Create Example]

   //! [Demand Query Auction Use Example]
   auto get_price = [](Item item) { return item == "honey" ? 5 : 2; };

   std::cout << "pooh bear buys: ";
   auto got_pooh_bear = auction.call<pa::demand_query>("Pooh Bear", get_price);
   boost::copy(got_pooh_bear.first, std::ostream_iterator<Item>(std::cout, ", "));
   std::cout << std::endl;

   std::cout << "rabbit oracle buys: ";
   auto got_rabbit = auction.call<pa::demand_query>("Rabbit", get_price);
   boost::copy(got_rabbit.first, std::ostream_iterator<Item>(std::cout, ", "));
   std::cout << std::endl;
   //! [Demand Query Auction Use Example]
   return 0;
}
