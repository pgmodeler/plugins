//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file ids.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-07-24
 */

#ifndef PAAL_IDS_HPP
#define PAAL_IDS_HPP

#include <functional>

namespace paal {
namespace lp {

/**
 * Base identifier class.
 */
class Id {
  protected:
    /// Constructor.
    Id() {}
    /// Constructor.
    explicit Id(int id) : m_id(id) {}

  public:
    /// Returns the id number.
    int get() const { return m_id; }

    /// Less operator.
    bool operator<(Id id) const { return m_id < id.m_id; }

    /// Equality operator.
    bool operator==(Id id) const { return m_id == id.m_id; }

  private:
    int m_id;
};

/**
 * LP column identifier class.
 */
struct col_id : Id {
    col_id() {}
    /// Constructor.
    explicit col_id(int id) : Id(id) {}
};

/**
 * LP row identifier class.
 */
struct row_id : Id {
    row_id() {}
    /// Constructor.
    explicit row_id(int id) : Id(id) {}
};

} // lp
} // paal

namespace std {

/**
 * Hash function for Id class.
 */
template <> struct hash<paal::lp::Id> {
    /**
     * Returns the hash of a given Id object.
     */
    std::size_t operator()(const paal::lp::Id &x) const {
        return hash<int>()(x.get());
    }
};

template <> struct hash<paal::lp::row_id> : public hash<paal::lp::Id> {};

template <> struct hash<paal::lp::col_id> : public hash<paal::lp::Id> {};

} // std

#endif // PAAL_IDS_HPP
