/*
 * @file winner_determination_in_MUCA.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-1-7
 */
#ifndef PAAL_WINNER_DETERMINATION_IN_MUCA_HPP
#define PAAL_WINNER_DETERMINATION_IN_MUCA_HPP

#include "paal/auctions/auction_components.hpp"
#include "paal/auctions/auction_traits.hpp"
#include "paal/auctions/auction_utils.hpp"
#include "paal/utils/concepts.hpp"
#include "paal/utils/property_map.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/concept/requires.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/concepts.hpp>
#include <boost/range/iterator.hpp>

#include <iterator>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace paal {
namespace auctions {

namespace detail {
   template <class Value, class ItemSet>
   struct bidder_info {
      Value m_best_items_val;
      ItemSet m_best_items;
   };

   template <
      class GammaOracleAuction,
      class Base = gamma_oracle_auction_traits<GammaOracleAuction>
   >
   struct determine_winners_in_gamma_oracle_auction_traits : Base {
         using value = promote_with_double_t<typename Base::value_t>;
         using item_set = typename Base::items_t;
         using bidder_info = detail::bidder_info<value, item_set>;
         using result = std::pair<typename Base::bidder_t, item_set>;
   };
}//!detail

/**
 * @brief This is determine winners in gamma oracle auction and return assignment of items to bidders.
 *
 * Example:
 *  \snippet winner_determination_in_MUCA_example.cpp Winner Determination In MUCA Example
 *
 * Complete example is winner_determination_in_MUCA_example.cpp.
 *
 * @tparam GammaOracleAuction
 * @tparam OutputIterator
 * @tparam PriceMap
 * @tparam Epsilon
 * @param auction
 * @param result
 * @param price Stores the current mapping of items to prices.
 * These are prices just for the working purposes of the algorithm,
 * not the prices to be paid by the bidders.
 * @param epsilon Used for floating point comparison to ensure feasibility.
 */
template<
   class GammaOracleAuction,
   class OutputIterator,
   class PriceMap,
   class Epsilon
>
BOOST_CONCEPT_REQUIRES(
   ((concepts::gamma_oracle_auction<GammaOracleAuction>))

   ((boost::ForwardRangeConcept<
     typename gamma_oracle_auction_traits<GammaOracleAuction>::bidders_universe_t
   >))

   ((utils::concepts::move_constructible<
     typename gamma_oracle_auction_traits<GammaOracleAuction>::items_t
   >))

   ((utils::concepts::output_iterator<
      OutputIterator,
      std::pair<
         typename gamma_oracle_auction_traits<GammaOracleAuction>::bidder_t,
         typename gamma_oracle_auction_traits<GammaOracleAuction>::items_t
      >
   >))

   ((boost::ReadWritePropertyMapConcept<
      PriceMap,
      typename gamma_oracle_auction_traits<GammaOracleAuction>::item_t
   >))

   ((utils::concepts::floating_point<Epsilon>)),

(void))
determine_winners_in_gamma_oracle_auction(
   GammaOracleAuction&& auction,
   OutputIterator result,
   PriceMap price,
   Epsilon epsilon
) {
   using Price = typename boost::property_traits<PriceMap>::value_type;

   using Traits =
      detail::determine_winners_in_gamma_oracle_auction_traits<GammaOracleAuction>;
   using Value = typename Traits::value;
   using BidderInfo = typename Traits::bidder_info;
   using BidderIter = typename Traits::bidder_iterator_t;
   using Frac = typename Traits::frac_t;
   using Res = typename Traits::result_t;

   Price items_num = 0;
   for (auto item = std::begin(auction.template get<items>());
         item != std::end(auction.template get<items>());
         ++item
   ) {
      ++items_num;
      auto const copies = auction.template call<get_copies_num>(*item);
      put(price, *item, 1.0 / copies);
   }
   Price price_sum = items_num;

   // TODO allow to change the allocator
   std::vector<BidderInfo> bidders_info_vec(bidders_number(auction));

   auto last_assigned_bidder_info = bidders_info_vec.end();
   BidderIter last_assigned_bidder;
   Value total_value = 0, last_value{};
   auto const b = get_minimum_copies_num(auction);
   auto const multiplier = std::exp(Value(b) + 1) * items_num;
   auto const gamma_ = auction.template get<gamma>();
   auto get_threshold = [=](const BidderInfo& b)
   {
      return (1 + 2 * gamma_) * b.m_best_items_val;
   };
   do {
      Res best = boost::none;
      auto get_frac = [](Res r) { return r->second; };
      auto bidder_info = bidders_info_vec.begin();
      auto bidder = std::begin(auction.template get<bidders>());
      for (; bidder_info != bidders_info_vec.end(); ++bidder_info, ++bidder) {
         auto const threshold = get_threshold(*bidder_info);
         auto result = auction.template call<gamma_oracle>(
            *bidder, utils::make_property_map_get(price), threshold
         );
         if (!result) continue;
         if (!best || get_frac(result) < get_frac(best)) {
            best = std::move(result);
            last_assigned_bidder_info = bidder_info;
            last_assigned_bidder = bidder;
            last_value = get_frac(result).den + threshold;
         }
      }
      if (!best) break;
      auto& best_items = best->first;
      for (auto item = std::begin(best_items); item != std::end(best_items); ++item) {
         auto const copies = auction.template call<get_copies_num>(*item);
         auto const old_price = get(price, *item);
         auto const new_price =
            old_price * std::pow(multiplier, 1.0 / (copies+ 1));
         put(price, *item, new_price);
         price_sum += copies* (new_price - old_price);
      }
      total_value += last_value - last_assigned_bidder_info->m_best_items_val;
      last_assigned_bidder_info->m_best_items = std::move(best_items);
      last_assigned_bidder_info->m_best_items_val = last_value;
   } while (price_sum + epsilon < multiplier);

   const bool nothing_assigned =
      last_assigned_bidder_info == bidders_info_vec.end();
   if (nothing_assigned) return;

   auto output = [&](
      puretype(last_assigned_bidder_info) bidder_info,
      BidderIter bidder
   ) {
      *result = std::make_pair(*bidder, std::move(bidder_info->m_best_items));
      ++result;
   };
   if (last_value > total_value - last_value) {
      output(last_assigned_bidder_info, last_assigned_bidder);
      return;
   }
   auto bidder_info = bidders_info_vec.begin();
   auto bidder = std::begin(auction.template get<bidders>());
   for (; bidder_info != bidders_info_vec.end(); ++bidder_info, ++bidder)
      if (bidder != last_assigned_bidder)
         output(bidder_info, bidder);
}

/**
 * @brief This is determine winners in gamma oracle auction and return assignment of bidders to items.
 * This is version with default PriceMap using std::unordered_map and
 * default epsilon.
 *
 * @tparam GammaOracleAuction
 * @tparam OutputIterator
 * @tparam Epsilon
 * @param auction
 * @param result
 * @param epsilon Used for floating point comparison to ensure feasibility.
 */
template <
   class GammaOracleAuction,
   class OutputIterator,
   class Epsilon = double
>
void determine_winners_in_gamma_oracle_auction(
   GammaOracleAuction&& auction,
   OutputIterator result,
   Epsilon epsilon = 1e-8
) {
   using Traits = gamma_oracle_auction_traits<GammaOracleAuction>;
   using Value = promote_with_double_t<typename Traits::value_t>;
   using ItemVal = typename Traits::item_val_t;

   std::unordered_map<ItemVal, Value> umap;
   return determine_winners_in_gamma_oracle_auction(
      std::forward<GammaOracleAuction>(auction),
      result,
      boost::make_assoc_property_map(umap),
      epsilon
   );
}


}//!auctions
}//!paal

#endif /* PAAL_WINNER_DETERMINATION_IN_MUCA_HPP */
