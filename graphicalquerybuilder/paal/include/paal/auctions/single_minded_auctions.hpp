//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file single_minded_auctions.hpp
 * @brief Interfaces for creating auctions from single minded valuations.
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-01-08
 */
#ifndef PAAL_SINGLE_MINDED_AUCTIONS_HPP
#define PAAL_SINGLE_MINDED_AUCTIONS_HPP

#include "paal/auctions/xor_bids.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/singleton_iterator.hpp"

#include <boost/range/iterator_range.hpp>

#include <utility>

namespace paal {
namespace auctions {

   namespace concepts {
      template <
         class Bidders,
         class Items,
         class GetValue,
         class GetItems,
         class GetCopiesNum
      >
      class single_minded {
         Bidders bidders;
         Items items;
         GetValue get_value;
         GetItems get_items;
         GetCopiesNum get_copies_num;

         single_minded() {}

         public:
            BOOST_CONCEPT_USAGE(single_minded)
            {
               using value_t = puretype(get_value(*std::begin(bidders)));
               static_assert(std::is_arithmetic<value_t>::value,
                     "get_value return type is not arithmetic!");
               auto&& bid_items = get_items(*std::begin(bidders));
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

      struct get_bids {
         template <class Bidder>
         auto operator()(Bidder&& b)
         const -> decltype(utils::make_singleton_range(std::forward<Bidder>(b)))
         {
            return utils::make_singleton_range(std::forward<Bidder>(b));
         }
      };

   } //!detail

   /**
    * @brief Create value query auction from single minded valuations.
    *
    * @param bidders
    * @param items
    * @param get_value
    * @param get_items
    * @param get_copies_num
    * @tparam Bidders
    * @tparam Items
    * @tparam GetValue
    * @tparam GetItems
    * @tparam GetCopiesNum
    */
   template<
      class Bidders,
      class Items,
      class GetValue,
      class GetItems,
      class GetCopiesNum = utils::return_one_functor
   >
   auto make_single_minded_to_value_query_auction(
      Bidders&& bidders,
      Items&& items,
      GetValue get_value,
      GetItems get_items,
      GetCopiesNum get_copies_num = GetCopiesNum{}
   )
   -> decltype(make_xor_bids_to_value_query_auction(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::get_bids(),
         get_value,
         get_items,
         get_copies_num
   )) {
      BOOST_CONCEPT_ASSERT((concepts::single_minded<Bidders, Items, GetValue, GetItems, GetCopiesNum>));
      return make_xor_bids_to_value_query_auction(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::get_bids(),
         get_value,
         get_items,
         get_copies_num
      );
   }

   // TODO all constructions in this file are essentially the same, maybe it's possible
   // to refactor it using some C++ magic?

   /**
    * @brief Create demand query auction from single minded valuations.
    *
    * @param bidders
    * @param items
    * @param get_value
    * @param get_items
    * @param get_copies_num
    * @tparam Bidders
    * @tparam Items
    * @tparam GetValue
    * @tparam GetItems
    * @tparam GetCopiesNum
    */
   template<
      class Bidders,
      class Items,
      class GetValue,
      class GetItems,
      class GetCopiesNum = utils::return_one_functor
   >
   auto make_single_minded_to_demand_query_auction(
      Bidders&& bidders,
      Items&& items,
      GetValue get_value,
      GetItems get_items,
      GetCopiesNum get_copies_num = GetCopiesNum{}
   )
   -> decltype(make_xor_bids_to_demand_query_auction(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::get_bids(),
         get_value,
         get_items,
         get_copies_num
   )) {
      BOOST_CONCEPT_ASSERT((concepts::single_minded<Bidders, Items, GetValue, GetItems, GetCopiesNum>));
      return make_xor_bids_to_demand_query_auction(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::get_bids(),
         get_value,
         get_items,
         get_copies_num
      );
   }

   /**
    * @brief Create gamma oracle auction from single minded valuations.
    *
    * @param bidders
    * @param items
    * @param get_value
    * @param get_items,
    * @param get_copies_num
    * @tparam Bidders
    * @tparam Items
    * @tparam GetValue
    * @tparam GetItems
    * @tparam GetCopiesNum
    */
   template<
      class Bidders,
      class Items,
      class GetValue,
      class GetItems,
      class GetCopiesNum = utils::return_one_functor
   >
   auto make_single_minded_to_gamma_oracle_auction(
      Bidders&& bidders,
      Items&& items,
      GetValue get_value,
      GetItems get_items,
      GetCopiesNum get_copies_num = GetCopiesNum{}
   )
   -> decltype(make_xor_bids_to_gamma_oracle_auction(
      std::forward<Bidders>(bidders),
      std::forward<Items>(items),
      detail::get_bids(),
      get_value,
      get_items,
      get_copies_num
   )) {
      BOOST_CONCEPT_ASSERT((concepts::single_minded<Bidders, Items, GetValue, GetItems, GetCopiesNum>));
      return make_xor_bids_to_gamma_oracle_auction(
         std::forward<Bidders>(bidders),
         std::forward<Items>(items),
         detail::get_bids(),
         get_value,
         get_items,
         get_copies_num
      );
   }

   /**
    * @brief Extract all items appearing in all bidders' bids. This function
    * doesn't eliminate duplicates, this is left out to the caller.
    *
    * @param bidders
    * @param get_items
    * @param output
    * @tparam Bidders
    * @tparam GetItems
    * @tparam OutputIterator
    */
   template<class Bidders, class GetItems, class OutputIterator>
   void extract_items_from_single_minded(Bidders&& bidders, GetItems get_items, OutputIterator output)
   {
      extract_items_from_xor_bids(
         std::forward<Bidders>(bidders),
         detail::get_bids(),
         get_items,
         output
      );
   }

} //!auctions
} //!paal
#endif // PAAL_SINGLE_MINDED_AUCTIONS_HPP
