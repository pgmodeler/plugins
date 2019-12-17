//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file gamma_oracle_example.cpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-6-6
 */

#include "paal/auctions/auction_components.hpp"
#include "paal/data_structures/fraction.hpp"

#include <boost/optional/optional.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

//! [Gamma Oracle Auction Components Example]

namespace pa = paal::auctions;
namespace pds = paal::data_structures;

using Bidder = std::string;
using Item = std::string;
using Items = std::vector<Item>;
using Value = int;
using Frac = pds::fraction<Value, Value>;

const std::vector<Bidder> bidders {"Pooh Bear", "Rabbit"};

const Items items {"honey", "baby carrot", "carrot", "jam"};

const int gamma_val = 2;

struct gamma_oracle_func {
      template <class GetPrice, class Threshold>
      boost::optional<std::pair<Items, Frac>>
      operator()(Bidder bidder, GetPrice get_price, Threshold z) const {

          if (bidder == "Pooh Bear") {
              const Value val = 10;
              if (val <= z) return boost::none;
              return std::make_pair(Items{"honey"}, Frac(get_price("honey"), val - z));
          }

          assert(bidder == "Rabbit");

          const Value baby_val = 2, val = 3;
          auto const baby_price = get_price("baby carrot");
          auto const price = get_price("carrot");
          auto const baby_frac = Frac(baby_price, baby_val - z),
          frac = Frac(price, val - z),
          both_frac = Frac(baby_price + price, baby_val + val - z);

          auto check = [=](Frac candidate, Frac other1, Frac other2) {
            if (candidate.den <= 0) return false;
            auto check_single = [=](Frac candidate, Frac other) {
                 return other.den <= 0 || candidate <= gamma_val * other;
            };
            return check_single(candidate, other1) && check_single(candidate, other2);
          };

          if (check(baby_frac, frac, both_frac))
              return std::make_pair(Items{"baby carrot"}, baby_frac);
          if (check(frac, baby_frac, both_frac))
              return std::make_pair(Items{"carrot"}, frac);
          if (check(both_frac, baby_frac, frac))
              return std::make_pair(Items{"baby carrot", "carrot"}, both_frac);
          return boost::none;
      }
};

//! [Gamma Oracle Auction Components Example]

int main()
{
   //! [Gamma Oracle Auction Create Example]
   auto const auction = pa::make_gamma_oracle_auction_components(
      bidders, items, gamma_oracle_func(), gamma_val
   );
   //! [Gamma Oracle Auction Create Example]

   //! [Gamma Oracle Auction Use Example]
   auto get_price_func = [](Item item) { return item == "honey" ? 5 : 2; };

   std::cout << "pooh bear buys: ";
   auto got_pooh_bear =
      auction.call<pa::gamma_oracle>("Pooh Bear", get_price_func, 10);
   if (!got_pooh_bear)
      std::cout << "nothing";
   else
      boost::copy(got_pooh_bear->first, std::ostream_iterator<Item>(std::cout, ", "));
   std::cout << std::endl;

   std::cout << "rabbit oracle buys: ";
   auto got_rabbit = auction.call<pa::gamma_oracle>("Rabbit", get_price_func, 1);
   if (!got_rabbit)
      std::cout << "nothing";
   else
      boost::copy(got_rabbit->first, std::ostream_iterator<Item>(std::cout, ", "));
   std::cout << std::endl;

   //! [Gamma Oracle Auction Use Example]
   return 0;
}
