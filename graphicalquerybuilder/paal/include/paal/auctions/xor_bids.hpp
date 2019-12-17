//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file xor_bids.hpp
 * @brief Interfaces for creating auctions from xor bids valuations.
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-01-21
 */
#ifndef PAAL_XOR_BIDS_HPP
#define PAAL_XOR_BIDS_HPP

#include "paal/auctions/auction_components.hpp"
#include "paal/data_structures/fraction.hpp"
#include "paal/utils/accumulate_functors.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/concept_check.hpp>
#include <boost/concept/requires.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/iterator.hpp>

#include <iterator>
#include <type_traits>
#include <utility>

namespace detail {
   // forward declaration for making this class a friend of xor_bids_gamma_oracle
   template<class GetBids, class GetValue, class GetItems, class Gamma>
   class test_xor_bids_gamma_oracle;
}

namespace paal {
namespace auctions {

   namespace concepts {

      template <
         class Bidders,
         class Items,
         class GetBids,
         class GetValue,
         class GetItems,
         class GetCopiesNum
      >
      class xor_bids {
         Bidders bidders;
         Items items;
         GetBids get_bids;
         GetValue get_value;
         GetItems get_items;
         GetCopiesNum get_copies_num;

         xor_bids() {}

         public:
            BOOST_CONCEPT_USAGE(xor_bids)
            {
               auto&& bids = get_bids(*std::begin(bidders));
               BOOST_CONCEPT_ASSERT((boost::ForwardRangeConcept<
                        decltype(bids)>));
               auto bid = std::begin(bids);
               using value_t = puretype(get_value(*bid));
               static_assert(std::is_arithmetic<value_t>::value,
                     "get_value return type is not arithmetic!");
               auto&& bid_items = get_items(*bid);
               using bundle_t = puretype(bid_items);
               static_assert(std::is_move_constructible<bundle_t>::value,
                     "bundle_t is not move constructible!");
               static_assert(std::is_default_constructible<bundle_t>::value,
                     "bundle_t is not default constructible!");
               BOOST_CONCEPT_ASSERT((boost::ForwardRangeConcept<
                        decltype(bid_items)>));
            }
      };
   } //!concepts

   namespace detail {

      template <class Bidder, class GetBids, class GetValue, class GetItems>
      struct xor_bids_traits {
         using bid_iterator =
            typename boost::range_iterator<typename std::result_of<
               GetBids(Bidder)>::type>::type;
         using bid = typename std::iterator_traits<bid_iterator>::reference;
         using value = pure_result_of_t<GetValue(bid)>;
         using items = typename std::result_of<GetItems(bid)>::type;
         using items_val = typename std::decay<items>::type;
         using item = range_to_ref_t<items>;
         template <class GetPrice>
         using price = pure_result_of_t<GetPrice(item)>;
      };

      template <class GetBids, class GetValue, class GetItems>
      class xor_bids_value_query {

         GetBids m_get_bids;
         GetValue m_get_value;
         GetItems m_get_items;

         template <class Bidder>
         using traits = xor_bids_traits<Bidder, GetBids, GetValue, GetItems>;

         public:
            xor_bids_value_query(GetBids get_bids, GetValue get_value, GetItems get_items)
             : m_get_bids(get_bids), m_get_value(get_value), m_get_items(get_items) {}

            template <
               class Bidder,
               class ItemSet,
               class Traits = traits<Bidder>,
               class Value = typename Traits::value
            >
            Value operator()(Bidder&& bidder, const ItemSet& item_set) const
            {
               using Bid = typename Traits::bid;
               using Item = typename Traits::item;

               auto is_contained = [&](Bid b)
               {
                  return boost::algorithm::all_of(
                     m_get_items(std::forward<Bid>(b)),
                     [&](Item i) { return item_set.count(std::forward<Item>(i)) > 0; }
                  );
               };
               return accumulate_functor(
                  m_get_bids(std::forward<Bidder>(bidder)) |
                     boost::adaptors::filtered(is_contained),
                  Value(0),
                  m_get_value,
                  paal::utils::max()
               );
            }
      };
   }; //!detail

   /**
    * @brief Create value query auction from xor bids valuations.
    *
    * @param bidders
    * @param items
    * @param get_bids
    * @param get_value
    * @param get_items
    * @param get_copies_num
    * @tparam Bidders
    * @tparam Items
    * @tparam GetBids
    * @tparam GetValue
    * @tparam GetItems
    * @tparam GetCopiesNum
    */
   template<
      class Bidders,
      class Items,
      class GetBids,
      class GetValue,
      class GetItems,
      class GetCopiesNum = utils::return_one_functor
   >
   auto make_xor_bids_to_value_query_auction(
      Bidders&& bidders,
      Items&& items,
      GetBids get_bids,
      GetValue get_value,
      GetItems get_items,
      GetCopiesNum get_copies_num = GetCopiesNum{}
   ) ->
   decltype(make_value_query_auction_components(
      std::forward<Bidders>(bidders),
      std::forward<Items>(items),
      detail::xor_bids_value_query<GetBids, GetValue, GetItems>(get_bids, get_value, get_items),
      get_copies_num
   ))
   {
      BOOST_CONCEPT_ASSERT((concepts::xor_bids<Bidders, Items, GetBids, GetValue, GetItems, GetCopiesNum>));
      return make_value_query_auction_components(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::xor_bids_value_query<GetBids, GetValue, GetItems>(get_bids, get_value, get_items),
         get_copies_num
      );
   }

   namespace detail {

      template <class GetBids, class GetValue, class GetItems>
      class xor_bids_demand_query {

         GetBids m_get_bids;
         GetValue m_get_value;
         GetItems m_get_items;

         template <class Bidder>
         using traits = xor_bids_traits<Bidder, GetBids, GetValue, GetItems>;

         template <class Bidder, class GetPrice, class Base = traits<Bidder>>
         struct price_traits : Base {
            using price = typename Base::template price<GetPrice>;
            using utility = promote_with_t<price, typename Base::value>;
         };

         public:
            xor_bids_demand_query(GetBids get_bids, GetValue get_value,
                  GetItems get_items) : m_get_bids(get_bids),
            m_get_value(get_value), m_get_items(get_items) {}

         template <class Bidder, class GetPrice>
         auto operator()(Bidder&& bidder, GetPrice get_price) const
         {
            using Traits = price_traits<Bidder, GetPrice>;
            using Items = typename Traits::items_val;
            using Res = std::pair<Items, typename Traits::utility>;

            Res best = {Items{}, 0};
            auto&& bids = m_get_bids(std::forward<Bidder>(bidder));
            for (auto bid = std::begin(bids); bid != std::end(bids); ++bid) {
               auto const value = m_get_value(*bid);
               auto const price =
                  sum_functor(m_get_items(*bid), get_price);
               auto const util = value - price;
               if (util > best.second)
                  best = {m_get_items(*bid), util};
            }
            return best;
         }
      };

   } //!detail

   /**
    * @brief Create demand query auction from xor bids valuations.
    *
    * @param bidders
    * @param items
    * @param get_bids
    * @param get_value
    * @param get_items
    * @param get_copies_num
    * @tparam Bidders
    * @tparam Items
    * @tparam GetBids
    * @tparam GetValue
    * @tparam GetItems
    * @tparam GetCopiesNum
    */
   template<
      class Bidders,
      class Items,
      class GetBids,
      class GetValue,
      class GetItems,
      class GetCopiesNum = utils::return_one_functor
   >
   auto make_xor_bids_to_demand_query_auction(
      Bidders&& bidders,
      Items&& items,
      GetBids get_bids,
      GetValue get_value,
      GetItems get_items,
      GetCopiesNum get_copies_num = GetCopiesNum{})
   {
      BOOST_CONCEPT_ASSERT((concepts::xor_bids<Bidders, Items, GetBids,
               GetValue, GetItems, GetCopiesNum>));
      return make_demand_query_auction_components(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::xor_bids_demand_query<GetBids, GetValue, GetItems>(get_bids,
            get_value, get_items),
         get_copies_num
      );
   }

   namespace detail {

      template<class GetBids, class GetValue, class GetItems>
      class xor_bids_gamma_oracle {

         GetBids m_get_bids;
         GetValue m_get_value;
         GetItems m_get_items;

         template <
            class GetBids_,
            class GetValue_,
            class GetItems_,
            class Gamma_
         >
         friend class ::detail::test_xor_bids_gamma_oracle;

         template <class Bidder>
         using traits = xor_bids_traits<Bidder, GetBids, GetValue, GetItems>;

         template <class Bidder, class GetPrice, class Base = traits<Bidder>>
         struct price_traits : public Base {
            using price = typename Base::template price<GetPrice>;
            using frac =
               paal::data_structures::fraction<price, typename Base::value>;
            using best_bid =
               boost::optional<std::pair<typename Base::bid_iterator, frac>>;
         };

         template <
            class Bidder,
            class GetPrice,
            class Threshold,
            class IsBetter,
            class BestBid = typename price_traits<Bidder, GetPrice>::best_bid
         >
         BestBid
         calculate_best(
            Bidder&& bidder,
            GetPrice get_price,
            Threshold threshold,
            IsBetter is_better
         ) const
         {
            BestBid result{};
            auto&& bids = m_get_bids(std::forward<Bidder>(bidder));
            for (auto bid = std::begin(bids); bid != std::end(bids); ++bid) {
               auto const value = m_get_value(*bid);
               if (value <= threshold) continue;
               auto const price = sum_functor(m_get_items(*bid),
                     get_price);
               auto const frac =
                  data_structures::make_fraction(price, value - threshold);
               if (is_better(frac, result))
                  result = std::make_pair(bid, frac);
            }
            return result;
         }

         template <
            class Bidder,
            class GetPrice,
            class Threshold,
            class Traits = price_traits<Bidder, GetPrice>,
            class BestBid = typename Traits::best_bid
         >
         BestBid
         minimum_frac(Bidder&& bidder, GetPrice get_price, Threshold threshold)
         const
         {
            return calculate_best(
               std::forward<Bidder>(bidder),
               get_price,
               threshold,
               [&](typename Traits::frac frac, const BestBid& result)
               {
                  return !result || frac < result->second;
               }
            );
         }

         template <class Result, class OutputIterator>
         auto output(const Result& result, OutputIterator out) const
         {
            auto bid = result.first;
            auto frac = result.second;
            boost::copy(m_get_items(*bid), out);
            return frac;
         }

         public:
            xor_bids_gamma_oracle(GetBids get_bids, GetValue get_value, GetItems get_items)
               : m_get_bids(get_bids), m_get_value(get_value), m_get_items(get_items) {}

            template <
               class Bidder,
               class GetPrice,
               class Threshold,
               class Traits = price_traits<Bidder, GetPrice>
            >
            boost::optional<std::pair<typename Traits::items_val, typename Traits::frac>>
            operator()(Bidder&& bidder, GetPrice get_price, Threshold threshold) const
            {
               auto const best = minimum_frac(std::forward<Bidder>(bidder),
                     get_price, threshold);
               if (!best) return boost::none;
               return std::make_pair(m_get_items(*best->first), best->second);
            }
      };
   }; //!detail

   /**
    * @brief Create gamma oracle auction from xor bids valuations.
    *
    * @param bidders
    * @param items
    * @param get_bids
    * @param get_value
    * @param get_items
    * @param get_copies_num
    * @tparam Bidders
    * @tparam Items
    * @tparam GetBids
    * @tparam GetValue
    * @tparam GetItems
    * @tparam GetCopiesNum
    */
   template<
      class Bidders,
      class Items,
      class GetBids,
      class GetValue,
      class GetItems,
      class GetCopiesNum = utils::return_one_functor
   >
   auto make_xor_bids_to_gamma_oracle_auction(
      Bidders&& bidders,
      Items&& items,
      GetBids get_bids,
      GetValue get_value,
      GetItems get_items,
      GetCopiesNum get_copies_num = GetCopiesNum{}
   )
   -> decltype(make_gamma_oracle_auction_components(
      std::forward<Bidders>(bidders),
      std::forward<Items>(items),
      detail::xor_bids_gamma_oracle<GetBids, GetValue, GetItems>(get_bids, get_value, get_items),
      1,
      get_copies_num
   ))
   {
      BOOST_CONCEPT_ASSERT((concepts::xor_bids<Bidders, Items, GetBids, GetValue, GetItems, GetCopiesNum>));
      return make_gamma_oracle_auction_components(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::xor_bids_gamma_oracle<GetBids, GetValue, GetItems>(get_bids, get_value, get_items),
         1,
         get_copies_num
      );
   }

   /**
    * @brief  extract all items appearing in all bids. This function
    * doesn't eliminate duplicates, this is left out to the caller.
    *
    * @tparam Bidders
    * @tparam GetBids
    * @tparam GetItems
    * @tparam OutputIterator
    * @param bidders
    * @param get_bids
    * @param get_items
    * @param output
    */
   template<class Bidders, class GetBids, class GetItems, class OutputIterator>
   void extract_items_from_xor_bids(
      Bidders&& bidders,
      GetBids get_bids,
      GetItems get_items,
      OutputIterator output
   ) {
      for (auto&& bidder: bidders) {
         for (auto&& bid: get_bids(std::forward<decltype(bidder)>(bidder))) {
            boost::copy(get_items(std::forward<decltype(bid)>(bid)), output);
         }
      }
   }

} //!auctions
} //!paal
#endif // PAAL_XOR_BIDS_HPP
