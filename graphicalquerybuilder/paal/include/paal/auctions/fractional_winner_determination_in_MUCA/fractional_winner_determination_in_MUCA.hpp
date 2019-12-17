//=======================================================================
// Copyright (c) 2013 Robert Rosolek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file fractional_winner_determination_in_MUCA.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-06-09
 */
#ifndef PAAL_FRACTIONAL_WINNER_DETERMINATION_IN_MUCA_HPP
#define PAAL_FRACTIONAL_WINNER_DETERMINATION_IN_MUCA_HPP

#include "paal/auctions/auction_components.hpp"
#include "paal/auctions/auction_traits.hpp"
#include "paal/auctions/auction_utils.hpp"
#include "paal/lp/glp.hpp"
#include "paal/lp/lp_row_generation.hpp"
#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/concepts.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/property_map.hpp"

#include <boost/concept/requires.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/counting_range.hpp>

#include <iterator>
#include <random>
#include <tuple>
#include <type_traits>

namespace paal {
namespace auctions {

namespace detail {

   template <class Bidder, class BidId, class Bundle>
   struct bid {
      Bidder m_bidder;
      BidId m_bid_id;
      Bundle m_bundle;
      bid(Bidder bidder, BidId bid_id, Bundle bundle) :
         m_bidder(bidder), m_bid_id(bid_id), m_bundle(bundle) {}
   };
}//! detail


/**
 * @brief This is fractional determine winners in demand query auction and return
 * assignment of fractional bundles to bidders.
 *
 * Example:
 *  \snippet fractional_winner_determination_in_MUCA_example.cpp
 *
 * Complete example is fractional_winner_determination_in_MUCA_example.cpp
 *
 * @tparam DemandQueryAuction
 * @tparam OutputIterator
 * @tparam ItemToLpIdMap
 * @tparam SeparationOracle
 * @param auction
 * @param result
 * @param item_to_id Stores the current mapping of items to LP column ids.
 * @param epsilon Used for floating point comparison.
 * @param separation_oracle Separation Oracle Strategy for searching the
 *  bidder with violated inequality.
 */
template <
   class DemandQueryAuction,
   class OutputIterator,
   class ItemToLpIdMap,
   class SeparationOracle = paal::lp::random_violated_separation_oracle
>
BOOST_CONCEPT_REQUIRES(

   ((concepts::demand_query_auction<DemandQueryAuction>))

   ((boost::ForwardRangeConcept<
     typename demand_query_auction_traits<DemandQueryAuction>::bidders_universe_t
   >))

   ((boost::ForwardRangeConcept<
     typename demand_query_auction_traits<DemandQueryAuction>::items_t
   >))

   ((utils::concepts::move_constructible<
     typename demand_query_auction_traits<DemandQueryAuction>::items_t
   >))

   ((utils::concepts::output_iterator<
     OutputIterator,
     std::tuple<
         typename demand_query_auction_traits<DemandQueryAuction>::bidder_t,
         typename demand_query_auction_traits<DemandQueryAuction>::items_t,
         double
      >
   >))

   ((boost::ReadWritePropertyMapConcept<
      ItemToLpIdMap,
      typename demand_query_auction_traits<DemandQueryAuction>::item_t
   >)),

   // TODO concept check for SeparationOracle

(void))
fractional_determine_winners_in_demand_query_auction(
   DemandQueryAuction&& auction,
   OutputIterator result,
   ItemToLpIdMap item_to_id,
   double epsilon,
   SeparationOracle separation_oracle = SeparationOracle{}
) {
   using traits_t = demand_query_auction_traits<DemandQueryAuction>;
   using bundle_t = typename traits_t::items_t;
   using bidder_t = typename traits_t::bidder_t;
   using bid_t = detail::bid<bidder_t, lp::row_id, bundle_t>;
   using result_t = typename traits_t::result_t;

   lp::glp dual;
   dual.set_optimization_type(lp::MINIMIZE);

   // add items variables to the dual
   auto&& items_ = auction.template get<items>();
   for (auto item = std::begin(items_); item != std::end(items_); ++item) {
      auto const copies = auction.template call<get_copies_num>(*item);
      auto const id = dual.add_column(copies, 0, lp::lp_traits::PLUS_INF, "");
      put(item_to_id, *item, id);
   }

   // add bidders variables to the dual
   // TODO allow to change the allocator
   std::vector<lp::col_id> bidder_to_id(bidders_number(auction));
   for (auto& id: bidder_to_id)
      id = dual.add_column(1, 0, lp::lp_traits::PLUS_INF, "");

   // TODO allow to change the allocator
   std::vector<bid_t> generated_bids;

   auto item_to_id_func = utils::make_property_map_get(item_to_id);
   auto get_price = utils::compose(
      [&](lp::col_id id) { return dual.get_col_value(id); },
      item_to_id_func
   );

   boost::optional<result_t> res;
   boost::optional<bidder_t> last_bidder;

   auto how_much_violated =
      utils::make_tuple_uncurry([&](bidder_t bidder, lp::col_id bidder_id)
   {
      //check if there is a violated constraint for bidder
      last_bidder = bidder;
      res = auction.template call<demand_query>(bidder, get_price);
      auto const util = res->second;
      auto const alpha = util - dual.get_col_value(bidder_id);
      if (alpha > epsilon) return boost::optional<double>(alpha);
      return boost::optional<double>{};
   });

   auto add_violated =
      utils::make_tuple_uncurry([&](bidder_t bidder, lp::col_id bidder_id) {
      assert(last_bidder);
      if (bidder != *last_bidder) {
        res = auction.template call<demand_query>(bidder, get_price);
      }

      auto& items = res->first;
      auto const util = res->second;

      // add violated constraint
      auto const price = sum_functor(items, get_price);
      auto const value = util + price;
      auto const expr = accumulate_functor(items,
            lp::linear_expression(bidder_id), item_to_id_func);
      auto const bid_id = dual.add_row(expr >= value);
      generated_bids.emplace_back(bidder, bid_id, std::move(items));
   });

   auto get_candidates = utils::make_dynamic_return_constant_functor(
      boost::combine(auction.template get<bidders>(), bidder_to_id));

   // TODO check if max_violated strategy doesn't give better performance
   auto find_violated = separation_oracle(get_candidates, how_much_violated, add_violated);

   auto solve_lp = [&]()
   {
      auto const res = dual.resolve_simplex(lp::DUAL);
      assert(res == lp::OPTIMAL);
      return res;
   };

   paal::lp::row_generation(find_violated, solve_lp);

   // emit results
   for (auto& bid: generated_bids) {
      auto const fraction = dual.get_row_dual_value(bid.m_bid_id);
      if (fraction <= epsilon) continue;
      *result = std::make_tuple(std::move(bid.m_bidder),
            std::move(bid.m_bundle), fraction);
      ++result;
   }
}

/**
 * @brief This is fractional determine winners in demand query auction and return
 * assignment of fractional bundles to bidders.
 * This is version with default ItemToLpIdMap using std::unordered_map and
 * default epsilon.
 *
 * @tparam DemandQueryAuction
 * @tparam OutputIterator
 * @param auction
 * @param result
 * @param epsilon Used for floating point comparison.
 */
template <class DemandQueryAuction, class OutputIterator>
void fractional_determine_winners_in_demand_query_auction(
   DemandQueryAuction&& auction,
   OutputIterator result,
   double epsilon = 1e-7
) {
   using traits_t = demand_query_auction_traits<DemandQueryAuction>;
   using ItemVal = typename traits_t::item_val_t;

   std::unordered_map<ItemVal, lp::col_id> map;
   return fractional_determine_winners_in_demand_query_auction(
      std::forward<DemandQueryAuction>(auction),
      result,
      boost::make_assoc_property_map(map),
      epsilon
   );
}

}//!auctions
}//!paal

#endif /* PAAL_FRACTIONAL_WINNER_DETERMINATION_IN_MUCA_HPP */
