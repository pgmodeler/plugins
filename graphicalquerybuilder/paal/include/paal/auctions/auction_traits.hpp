/**
 * @file auction_traits.hpp
 * @brief
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-03-24
 */
#ifndef PAAL_AUCTION_TRAITS_HPP
#define PAAL_AUCTION_TRAITS_HPP

#include "paal/auctions/auction_components.hpp"
#include "paal/data_structures/fraction.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/type_functions.hpp"

#include <boost/optional/optional.hpp>

#include <iterator>
#include <unordered_set>
#include <utility>

namespace paal {
namespace auctions {

/**
 * @brief Types associated with all auctions.
 *
 * @tparam Auction
 */
template <class Auction>
struct auction_traits {
   using bidders_universe_t =
      decltype(std::declval<Auction>().template get<bidders>());
   using bidder_iterator_t =
       typename boost::range_iterator<bidders_universe_t>::type;
   using bidder_t = range_to_ref_t<bidders_universe_t>;
   using bidder_val_t = range_to_elem_t<bidders_universe_t>;
   using items_universe_t =
      decltype(std::declval<Auction>().template get<items>());
   using item_t = range_to_ref_t<items_universe_t>;
   using item_val_t = range_to_elem_t<items_universe_t>;
   using copies_num_t = puretype(
      std::declval<Auction>().template call<get_copies_num>(
         std::declval<item_t>()
      )
   );
};

/**
 * @brief Types associated with value query auction.
 *
 * @tparam ValueQueryAuction
 */
template <class ValueQueryAuction>
class value_query_auction_traits: public auction_traits<ValueQueryAuction> {
   using base = auction_traits<ValueQueryAuction>;

   public:
      using value_t = puretype(std::declval<const ValueQueryAuction&>().template call<value_query>(
         std::declval<typename base::bidder_t>(),
         std::unordered_set<typename base::item_val_t>() // any container of items with count method
      ));
};

/**
 * @brief Types associated with demand query auction.
 *
 * @tparam DemandQueryAuction
 */
template <class DemandQueryAuction>
struct demand_query_auction_traits : public auction_traits<DemandQueryAuction> {

    using result_t = puretype(
            std::declval<DemandQueryAuction>().template call<demand_query>(
                std::declval<typename auction_traits<DemandQueryAuction>::bidder_t>(),
                // this is a little tricky, in order to obtain the value type, we pass prices and threshold
                // as double types, because value type needs to be able to operate with doubles anyway
                utils::make_dynamic_return_constant_functor(double(1.0)) // any functor with double operator()
                )
            );
    using items_t = typename result_t::first_type;
    using value_t = typename result_t::second_type;
};

/**
 * @brief Types associated with gamma oracle auction.
 *
 * @tparam GammaOracleAuction
 */
template <class GammaOracleAuction>
class gamma_oracle_auction_traits: public auction_traits<GammaOracleAuction> {
   using temp_result_t = puretype(
         *std::declval<GammaOracleAuction>(). template call<gamma_oracle>(
            std::declval<typename auction_traits<GammaOracleAuction>::bidder_t>(),
            // this is a little tricky, in order to obtain the value type, we pass prices
            // as double types, because value type needs to be able to operate with doubles anyway
            utils::make_dynamic_return_constant_functor(double(1.0)), // any functor with double operator()
            double(1.0) // any double
         )
      );

   public:
      using items_t = typename temp_result_t::first_type;
      using value_t = typename temp_result_t::second_type::den_type;
      using frac_t = data_structures::fraction<value_t, value_t>;
      using result_t = boost::optional<std::pair<items_t, frac_t>>;
};

} //!auctions
} //!paal
#endif // PAAL_AUCTION_TRAITS_HPP
