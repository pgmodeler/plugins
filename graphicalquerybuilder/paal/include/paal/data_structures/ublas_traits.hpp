//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file ublas_traits.hpp
 * @brief
 * @author Tomasz Strozak
 * @version 1.0
 * @date 2015-07-09
 */
#ifndef PAAL_UBLAS_TRAITS_HPP
#define PAAL_UBLAS_TRAITS_HPP

#include "paal/utils/type_functions.hpp"

#include <boost/numeric/bindings/traits/traits.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/ublas/banded.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace paal {
namespace data_structures {

template <typename RowType,
          typename Enable = void>
struct is_sparse_row : public std::false_type {};

template <typename RowType>
struct is_sparse_row<RowType,
    typename std::enable_if<
        std::is_same<typename paal::decay_t<RowType>::container_type::storage_category,
                     boost::numeric::ublas::sparse_tag>::value>::type> :
                public std::true_type {};

/// Traits class for matrix related types.
template <typename Matrix>
struct matrix_type_traits {};

/// Specialization matrix_type_traits for ublas matrix.
template <typename T>
struct matrix_type_traits<boost::numeric::ublas::matrix<T>> {
    using coordinate_t = T;
    using matrix_row_t = boost::numeric::ublas::matrix_row<boost::numeric::ublas::matrix<T>>;
    using vector_t = boost::numeric::ublas::vector<T>;
    using matrix_column_major_t = boost::numeric::ublas::matrix<T, boost::numeric::ublas::column_major>;
    using matrix_diagonal_t = boost::numeric::ublas::banded_matrix<T>;
    /// Return the number of rows of the matrix
    static std::size_t num_rows (boost::numeric::ublas::matrix<T> &m) { return m.size1(); }
    /// Return the number of columns of the matrix
    static std::size_t num_columns (boost::numeric::ublas::matrix<T> &m) { return m.size2(); }
};

} // data_structures
} // paal

#endif // PAAL_UBLAS_TRAITS_HPP
