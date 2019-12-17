//=======================================================================
// Copyright (c) 2014 Andrzej Pacuk
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file read_svm.hpp
 * @brief
 * @author Andrzej Pacuk
 * @version 1.0
 * @date 2014-10-22
 */
#ifndef PALL_READ_SVM_HPP
#define PALL_READ_SVM_HPP

#include "paal/utils/assign_updates.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/parse_file.hpp"
#include "paal/utils/type_functions.hpp"

#include <algorithm>
#include <cassert>
#include <istream>
#include <ios>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

namespace paal {

namespace detail {
/**
 * @brief resize rows to have equal sizes
 *
 * @tparam RowsRange
 * @tparam RowRefExtractor
 * @param rows
 * @param row_ref_extractor
 * @param new_size
 */
template <typename RowsRange, typename RowRefExtractor>
void resize_rows(RowsRange &&rows, RowRefExtractor row_ref_extractor,
                 std::size_t new_size) {
    for (auto &row : rows) {
        row_ref_extractor(row).resize(new_size);
    }
}



/**
 * @brief class that can read single svm row
 *
 * @tparam CoordinatesType
 * @tparam ResultType
 * @tparam FeatureIdType
 */
template <typename CoordinatesType,
          typename ResultType = int,
          typename FeatureIdType = std::size_t>
class svm_row {
    CoordinatesType m_coordinates;
    ResultType m_result;
    bool m_has_out_of_bounds_feature_id = false;
    using coordinate_t = range_to_elem_t<CoordinatesType>;

public:
    ///constructor
    svm_row(FeatureIdType dimensions = 1) : m_coordinates(dimensions) {}

    /**
     * @brief reads svm row of format:
     *
     * line .=. result feature:value feature:value ... feature:value
     *
     * result .=. 1 | -1
     *
     * feature .=. positive integer
     *
     * value .=. float
     *
     * Results are converted to 0 and 1.
     *
     * Minimal feature id = 0.
     *
     * @param row_stream containing only one row (passing stream with more than one row will fail the stream)
     * @param row
     *
     * @return should return empty stream
     */
    friend std::istream& operator>>(std::istream &row_stream, svm_row &row) {
        row.nullify_coordinates();

        if (!row.read_result(row_stream)) {
            return row_stream;
        }

        while (row_stream.good()) {
            row.read_single_feature(row_stream);
        }

        row.fail_stream_if_out_of_bound_features(row_stream);

        return row_stream;
    }

    /// coordinates getter
    CoordinatesType const &get_coordinates() const { return m_coordinates; }
    /// result getter
    ResultType const &get_result() const { return m_result; }

private:
    void nullify_coordinates() {
        auto size = m_coordinates.size();
        m_coordinates.clear();
        m_coordinates.resize(size);
    }

    std::istream& read_result(std::istream &row_stream) {
        ResultType result;
        if (!(row_stream >> result)) {
            return row_stream;
        }
        m_result = (result == 1) ? 1 : 0;

        return row_stream;
    }

    std::istream& read_single_feature(std::istream &row_stream) {
        FeatureIdType feature_id;
        read_feature_id(row_stream, feature_id);
        if (!row_stream.good()) {
            return row_stream;
        }

        if (!skip_exact_character(row_stream, ':')) {
            return row_stream;
        }

        coordinate_t coordinate;
        if (!(row_stream >> coordinate)) {
            return row_stream;
        }

        save(feature_id, coordinate);
        return row_stream;
    }

    std::istream& read_feature_id(std::istream &stream, FeatureIdType &feature_id) const {
        stream >> feature_id;
        if (stream.fail() && stream.eof()) {
            flip_state(stream, std::ios::failbit);
        }
        return stream;
    }

    void flip_state(std::istream &stream, const std::ios::iostate &state) const {
        stream.clear(stream.rdstate() ^ state);
    }

    std::istream& skip_exact_character(std::istream &stream, char character) const {
        char c;
        stream.get(c);
        if (!stream.good() || c != character) {
            stream.setstate(std::ios::failbit);
        }
        return stream;
    }

    void save(FeatureIdType feature_id, coordinate_t coordinate) {
        if (m_coordinates.size() <= feature_id) {
            m_has_out_of_bounds_feature_id = true;
            m_coordinates.resize(feature_id + 1);
        }
        m_coordinates[feature_id] = coordinate;
    }

    void fail_stream_if_out_of_bound_features(std::istream &stream) const {
        if (m_has_out_of_bounds_feature_id) {
            stream.setstate(std::ios::failbit);
        }
    }
};

} //! detail

/**
 * @brief reads up to max_points_to_read svm rows,
 * updating max_dimensions on each row
 *
 * @tparam RowType
 * @tparam ResultType
 * @tparam ShouldIgnoreBadRow
 * @param input_stream
 * @param max_dimensions
 * @param points
 * @param max_points_to_read
 * @param should_ignore_bad_row
 */
template <typename RowType,
          typename ResultType = int,
          typename ShouldIgnoreBadRow = utils::always_false>
void read_svm(std::istream &input_stream,
              std::size_t &max_dimensions,
              std::vector<std::tuple<RowType, ResultType>> &points,
              std::size_t max_points_to_read,
              ShouldIgnoreBadRow &&should_ignore_bad_row = ShouldIgnoreBadRow{}) {
    assert(input_stream.good());

    detail::svm_row<RowType, ResultType> row{max_dimensions};
    std::string line;
    while ((max_points_to_read--) && std::getline(input_stream, line)) {
        std::stringstream row_stream(line);
        row_stream >> row;
        if (row_stream || !should_ignore_bad_row(line)) {
            assign_max(max_dimensions, row.get_coordinates().size());
            points.emplace_back(row.get_coordinates(),
                    row.get_result());
        }
    }
}

/**
 * @brief Function parses svm stream of format:
 *
 * @tparam RowType
 * @tparam ResultType
 * @param input_stream
 *
 * @return vector of tuples (point, max_dimensions), where each point is
 * tuple (RowType, result)
 */
template <typename RowType,
          typename ResultType = int>
auto read_svm(std::istream &input_stream) {
    assert(input_stream.good());

    using point_with_result_t = std::tuple<RowType, ResultType>;

    std::size_t max_dimensions = 0;
    std::size_t max_points_to_read = 1;
    std::vector<point_with_result_t> points;
    while (input_stream.good()) {
        read_svm(input_stream, max_dimensions, points, max_points_to_read);
    }
    detail::resize_rows(points, utils::tuple_get<0>(), max_dimensions);

    return std::make_tuple(points, max_dimensions);
}

} //! paal

#endif /* PALL_READ_SVM_HPP */

