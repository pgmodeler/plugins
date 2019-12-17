//=======================================================================
// Copyright (c) 2015
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file read_rows.hpp
 * @brief
 * @author Tomasz Strozak
 * @version 1.0
 * @date 2015-07-23
 */
#ifndef PAAL_READ_ROWS_HPP
#define PAAL_READ_ROWS_HPP

#include "paal/utils/functors.hpp"
#include "paal/utils/system_message.hpp"

#include <boost/range/size.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/istream_range.hpp>

#include <cassert>
#include <istream>
#include <string>
#include <sstream>
#include <vector>
#include <list>

namespace paal {

/**
 * @brief reads up to max_rows_to_read rows of size row_size
 *
 * @tparam RowType
 * @tparam ShouldIgnoreBadRow
 * @param input_stream
 * @param rows
 * @param row_size
 * @param max_rows_to_read
 * @param should_ignore_bad_row
 */
template <typename CoordinateType,
          typename RowType = std::vector<CoordinateType>,
          typename ShouldIgnoreBadRow = utils::always_true>
void read_rows(std::istream &input_stream,
               std::vector<RowType> &rows,
               std::size_t row_size,
               std::size_t max_rows_to_read,
               ShouldIgnoreBadRow &&should_ignore_bad_row = ShouldIgnoreBadRow{}) {
    std::string line;

    RowType row;
    row.reserve(row_size);

    while ((max_rows_to_read--) && std::getline(input_stream, line)) {
        row.clear();
        std::stringstream row_stream(line);

        boost::copy(boost::istream_range<CoordinateType>(row_stream), std::back_inserter(row));

        if((!row_stream.bad() && row_size == boost::size(row)) || !should_ignore_bad_row(line)) {
            rows.emplace_back(row);
        }
    }
}

/**
 * @brief reads up to max_rows_to_read rows, size is determine by first row
 *
 * @tparam CoordinateType
 * @tparam RowType
 * @tparam ShouldIgnoreBadRow
 * @tparam FailureMessage
 * @param input_stream
 * @param rows
 * @param max_rows_to_read
 * @param should_ignore_bad_row
 * @param failure_message
 */
template <typename CoordinateType,
          typename RowType = std::vector<CoordinateType>,
          typename ShouldIgnoreBadRow = utils::always_true,
          typename FailureMessage = utils::failure_message>
void read_rows_first_row_size(std::istream &input_stream,
               std::vector<RowType> &rows,
               std::size_t max_rows_to_read,
               ShouldIgnoreBadRow &&should_ignore_bad_row = ShouldIgnoreBadRow{},
               FailureMessage &&failure_message = FailureMessage{}) {
    if(!input_stream.good()) {
        failure_message("Input stream is broken");
    }

    read_rows<CoordinateType>(input_stream, rows, 0, 1, utils::always_false{});

    if(rows.empty()) {
        failure_message("Empty input data");
    }
    std::size_t const row_size = boost::size(rows.front());
    if(row_size <= 0) {
        failure_message("Empty first row");
    }

    read_rows<CoordinateType>(input_stream, rows, row_size, max_rows_to_read - 1, std::forward<ShouldIgnoreBadRow>(should_ignore_bad_row));
}

} //! paal

#endif /* PAAL_READ_ROWS_HPP */
