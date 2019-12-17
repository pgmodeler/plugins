//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file auction_components.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-01-07
 */
#ifndef PAAL_AUCTION_COMPONENTS_HPP
#define PAAL_AUCTION_COMPONENTS_HPP

#include "paal/data_structures/components/component_traits.hpp"
#include "paal/data_structures/components/components.hpp"
#include "paal/data_structures/components/components_join.hpp"
#include "paal/utils/concepts.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/concept_check.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/concepts.hpp>

#include <type_traits>
#include <unordered_set>
#include <utility>

namespace paal {
/// Auctions namespace
namespace auctions {

   // Base

   /**
    * @brief name for the bidders component
    */
   struct bidders;
   /**
    * @brief name for the items component
    */
   struct items;
   /**
    * @brief name for the get_copies_num component
    */
   struct get_copies_num;

   // Value Query Auction

   /**
    * @brief name for the value query component
    */
   struct value_query;

   // Demand Query Auction

   /**
    * @brief name for the demand query component
    */
   struct demand_query;

   // Gamma Oracle Auction

   /**
    * @brief name for the gamma oracle component
    */
   struct gamma_oracle;
   /**
    * @brief name for the gamma component
    */
   struct gamma;

/// Auctions Concepts namespace
namespace concepts {
   template <typename Auction>
   class auction {
      template <class Component>
      using component_to_range_t = typename std::remove_reference<
         typename data_structures::component_traits<
            typename std::decay<Auction>::type>::
               template type<Component>::type
         >::type;

      public:

         auction() = delete;

         using bidders_t = component_to_range_t<bidders>;
         BOOST_CONCEPT_ASSERT((boost::SinglePassRangeConcept<bidders_t>));
         BOOST_CONCEPT_ASSERT((utils::concepts::readable_range<bidders_t>));

         using items_t = component_to_range_t<items>;
         BOOST_CONCEPT_ASSERT((boost::SinglePassRangeConcept<items_t>));
         BOOST_CONCEPT_ASSERT((utils::concepts::readable_range<items_t>));
         using item_val_t = range_to_elem_t<items_t>;

         BOOST_CONCEPT_USAGE(auction)
         {
            auto copies = a.template call<get_copies_num>(
                  *std::begin(a.template get<items>()));
            using get_copies_num_result_t = puretype(copies);
            static_assert(std::is_integral<get_copies_num_result_t>::value,
                  "return type of get_copies_num is not integral!");
         }

      protected:
         Auction a;

         auto get_item() -> decltype(*std::begin(a.template get<items>()))
         {
            return *std::begin(a.template get<items>());
         }

         auto get_bidder() -> decltype(*std::begin(a.template get<bidders>()))
         {
            return *std::begin(a.template get<bidders>());
         }
   };

   template <typename ValueQueryAuction>
   class value_query_auction : public auction<ValueQueryAuction> {
      using base = auction<ValueQueryAuction>;

      public:

         BOOST_CONCEPT_USAGE(value_query_auction)
         {
            auto value_query_ = this->a.template get<value_query>();
            auto val = value_query_(this->get_bidder(), std::unordered_set<
               typename base::item_val_t>{this->get_item()});
            using value_query_result_t = puretype(val);
            static_assert(std::is_arithmetic<value_query_result_t>::value,
                  "return type of value_query is not arithmetic!");
         }
   };

   template <typename DemandQueryAuction>
   struct demand_query_auction : auction<DemandQueryAuction> {

      BOOST_CONCEPT_USAGE(demand_query_auction)
      {
         auto demand_query_ = this->a.template get<demand_query>();
         auto get_price = utils::return_one_functor();
         auto res = demand_query_(this->get_bidder(), get_price);
         using demand_query_result_items_t = decltype(res.first);
         BOOST_CONCEPT_ASSERT((boost::SinglePassRangeConcept<
                  demand_query_result_items_t>));
         BOOST_CONCEPT_ASSERT((utils::concepts::readable_range<
                  demand_query_result_items_t>));
         using demand_query_result_value_t = puretype(res.second);
         static_assert(std::is_arithmetic<demand_query_result_value_t>::value,
            "second member of the result from demand query oracle is not arithmetic!");
      }
   };

   template <typename GammaOracleAuction>
   struct gamma_oracle_auction : auction<GammaOracleAuction> {

      using gamma_t = typename data_structures::component_traits<
         typename std::decay<GammaOracleAuction>::type>::
            template type<gamma>::type;
      static_assert(std::is_arithmetic<gamma_t>::value,
            "gamma type is not arithmetic!");

      BOOST_CONCEPT_USAGE(gamma_oracle_auction)
      {
         auto gamma_oracle_ = this->a.template get<gamma_oracle>();
         auto get_price = utils::return_one_functor();
         auto threshold = 0.;
         auto res = gamma_oracle_(this->get_bidder(), get_price, threshold);
         if (res) {}
         if (!res) {}
         using gamma_oracle_result_items_t = decltype(res->first);
         BOOST_CONCEPT_ASSERT((boost::SinglePassRangeConcept<
                  gamma_oracle_result_items_t>));
         BOOST_CONCEPT_ASSERT((utils::concepts::readable_range<
                  gamma_oracle_result_items_t>));
         using gamma_oracle_result_price_t = puretype(res->second.num);
         static_assert(std::is_arithmetic<gamma_oracle_result_price_t>::value,
            "numerator of frac returned from gamma oracle is not arithmetic!");
         using gamma_oracle_result_value_t = puretype(res->second.den);
         static_assert(std::is_arithmetic<gamma_oracle_result_value_t>::value,
            "denominator of frac returned from gamma oracle is not arithmetic!");
      }
   };

} //!concepts

   // Base

   /**
    * @brief Definition for the components class representing an auction.
    * This class is not meant to be directly used, it is just a base for the
    * more specialized components interfaces.
    */
   using base_auction_components = data_structures::components<
      bidders,
      items,
      data_structures::NameWithDefault<get_copies_num, utils::return_one_functor>
   >;

   namespace detail {
      /// extend base auction components with other components.
      template <typename... Names>
      using add_to_base_auction =
         typename data_structures::join<
            base_auction_components,
            data_structures::components<Names...>
         >::type;
   }; //!detail

   // Value Query Auction

   /**
    * @brief definition for the components class for a value query auction.
    */
   using value_query_components = detail::add_to_base_auction<value_query>;

   /**
    * @brief value query auction components template alias
    *
    * @tparam Args
    */
   template <typename... Args>
   using value_query_auction_components = typename value_query_components::type<Args...>;

   /**
    * @brief make function for value query components
    *
    * @tparam Args
    * @param args
    *
    * @return value query components
    */
   template <typename... Args>
   auto make_value_query_auction_components(Args&&... args) ->
      decltype(value_query_components::make_components(std::forward<Args>(args)...))
   {
      auto res = value_query_components::make_components(std::forward<Args>(args)...);
      BOOST_CONCEPT_ASSERT((concepts::value_query_auction<decltype(res)>));
      return res;
   }

   // Demand Query Auction

   /**
    * @brief definition for the components class for a demand query auction
    */
   using demand_query_components = detail::add_to_base_auction<demand_query>;

   /**
    * @brief demand query auction components template alias
    *
    * @tparam Args
    */
   template <typename... Args>
   using demand_query_auction_components = typename demand_query_components::type<Args...>;

   /**
    * @brief make function for demand query components
    *
    * @tparam Args
    * @param args
    *
    * @return demand query components
    */
   template <typename... Args>
   auto make_demand_query_auction_components(Args&&... args)
   {
      auto res = demand_query_components::make_components(std::forward<Args>(args)...);
      BOOST_CONCEPT_ASSERT((concepts::demand_query_auction<decltype(res)>));
      return res;
   }

   // Gamma Oracle Auction

   /**
    * @brief definition for the components class for a gamma oracle auction.
    */
   using gamma_oracle_components = detail::add_to_base_auction<gamma_oracle, gamma>;

   /**
    * @brief gamma oracle auction components template alias
    *
    * @tparam Args
    */
   template <typename... Args>
   using gamma_oracle_auction_components = typename gamma_oracle_components::type<Args...>;

   /**
    * @brief make function for gamma oracle components
    *
    * @tparam Args
    * @param args
    *
    * @return gamma oracle components
    */
   template <typename... Args>
   auto make_gamma_oracle_auction_components(Args&&... args) ->
      decltype(gamma_oracle_components::make_components(std::forward<Args>(args)...))
   {
      auto res = gamma_oracle_components::make_components(std::forward<Args>(args)...);
      BOOST_CONCEPT_ASSERT((concepts::gamma_oracle_auction<decltype(res)>));
      return res;
   }

} //!auctions
} //!paal
#endif // PAAL_AUCTION_COMPONENTS_HPP
