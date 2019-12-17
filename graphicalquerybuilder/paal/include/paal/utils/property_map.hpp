//=======================================================================
// Copyright (c) 2013 Robert Rosolek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file property_map.hpp
 * @brief Utils for the Boost PropertyMap
 * @author Robert Rosolek
 * @version 1.0
 * @date 2014-07-17
 */
#ifndef PAAL_PROPERTY_MAP_HPP
#define PAAL_PROPERTY_MAP_HPP

#include "paal/utils/type_functions.hpp"

#include <utility>

namespace paal {
namespace utils {

   // TODO use this in include/paal/steiner_tree/zelikovsky_11_per_6.hpp
   /**
    * @brief Functor object for property map get.
    * This class exists because std::bind can't be used with polymorphic
    * functions and we don't have polymorphic lambdas yet to allow perfect
    * forwarding.
    *
    * @tparam Map
    */
   template<class Map>
   class property_map_get {
      Map m_map;

      public:

      /**
       * @brief constructor
       *
       * @param map
       */
      property_map_get(Map map) : m_map(map) {}

      /**
       * @brief operator
       *
       * @tparam Key
       * @param key
       *
       * @return
       */
      template <class Key>
      auto operator()(Key&& key) const ->
      puretype(get(m_map, std::forward<Key>(key)))
      {
         return get(m_map, std::forward<Key>(key));
      }
   };

   /**
    * @brief make for property_map_get
    *
    * @tparam Map
    * @param map
    *
    * @return
    */
   template <class Map>
   property_map_get<Map> make_property_map_get(Map map)
   {
      return property_map_get<Map>(map);
   }

} //! utils
} //! paal

#endif /* PAAL_PROPERTY_MAP_HPP */
