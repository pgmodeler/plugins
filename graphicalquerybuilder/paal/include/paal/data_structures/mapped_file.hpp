//=======================================================================
// Copyright (c) 2014 Karol Wegrzycki
//
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/**
 * @file mapped_file.hpp
 * @brief Interface for using mmaped files with threads.
 * @author Karol Wegrzycki
 * @version 1.0
 * @date 2014-12-17
 */

#ifndef PAAL_MAPPED_FILE_HPP
#define PAAL_MAPPED_FILE_HPP

#define BOOST_ERROR_CODE_HEADER_ONLY
#define BOOST_SYSTEM_NO_DEPRECATED

#include "paal/utils/type_functions.hpp"
#include "paal/utils/irange.hpp"

#include "paal/data_structures/thread_pool.hpp"

#include <boost/iostreams/device/mapped_file.hpp>

#include <string>
#include <vector>
#include <thread>

namespace paal {
namespace data_structures {

/**
 * @class mapped_file
 * @brief data structure that gets new lines for many threads
 *
 */
class mapped_file {
private:

    char const * m_current;
    char const * m_file_begin;
    char const * m_file_end;
    char const * m_chunk_suggested_end;

public:

    /**
     * @brief Initializes mmaped file with the specific chunk - so that every
     *        thread could use different part of the file.
     *
     * @param file mmap file pointer
     * @param file_size size of the mmaped file
     * @param chunk_index m_current chunk index
     * @param chunk_cnt number of the chunks (usually equal the number of the threads)
     */
    mapped_file(char const * file, size_t file_size, unsigned chunk_index, unsigned chunk_cnt):
                                                            mapped_file(file, file_size) {
        assert(chunk_cnt > 0);
        assert(chunk_index < chunk_cnt);
        m_current = m_file_begin + file_size * chunk_index / chunk_cnt;
        m_chunk_suggested_end = m_file_begin + file_size * (chunk_index + 1) / chunk_cnt;
        if (m_current > m_file_begin && *(m_current-1) != '\n') {
            get_line();
        }
    }

    /**
     * @brief Initializes mmaped file.
     *
     * @param file - mmap file pointer
     * @param file_size - size of the mmaped file
     */
    mapped_file(char const * file, size_t file_size) :
        m_current(file),
        m_file_begin(file),
        m_file_end(file+file_size),
        m_chunk_suggested_end(m_file_end) {}

    /**
     * @brief Gets line from the m_current file. Eof and End Of Chunk
     *        aren't checked here.
     *
     * @return copied line
     */
    std::string get_line() {
        auto result_begin = m_current;
        auto result_end = std::find(m_current, m_file_end, '\n');

        m_current = result_end + 1;
        return std::string(result_begin, result_end-result_begin);
    }

    /**
     * @brief is m_currently at the end of file
     */
    bool eof() const {
        return m_current >= m_file_end;
    }

    /**
     * @brief is m_currently at the end of requested part of the file
     */
    bool end_of_chunk() const {
        return m_current >= m_chunk_suggested_end;
    }
    /**
     * @brief Computes functor on every line of the file. It takes care of
     *        the chunks and end of file.
     *
     * @tparam Functor
     * @param f - Functor that should be computed
     *
     */
    template <typename Functor>
    void for_each_line(Functor f) {
        while (!eof() && !end_of_chunk()) {
            f(get_line());
        }
    }

};


/**
 * @brief for_every_line function provides basic functionality for processing
 *        text files quickly and clearly. Thanks to mmap() functionality it doesn't
 *        have to seek through file but it loads it to virtual memory instantly and
 *        uses only ram cache to do that. Furthermore file is split instantly - thanks
 *        to that it can be processed effectively using threads. Downside of using mmap
 *        is that this functionality will not work effectively if threads have small jobs
 *        to be done comparing reading the line charge.
 *        It's supposed to work with O(threads_count) memory usage but remember -
 *        RES (resident size) stands for how much memory of this process is loaded in
 *        physical memory, so file pages loaded in ram cache are added to that value.
 *
 * @tparam Functor = std::string -> Result
 * @param f - Functor that should be evaluated for every line in file
 * @param file_path - path to the file for which values should be computed
 * @param threads_count - default std::thread::hardware_concurrency()
 */
template <typename Functor>
auto for_each_line(Functor f, std::string const & file_path,
        unsigned threads_count = std::thread::hardware_concurrency()) {

    using results_t = std::vector<pure_result_of_t<Functor(std::string)>>;

    std::vector<results_t> results(threads_count);
    thread_pool threads(threads_count);

    boost::iostreams::mapped_file_source mapped(file_path);
    auto data = mapped.data();

    for (auto i : irange(threads_count)) {
        threads.post([&, i]() {
            mapped_file file_chunk(data, mapped.size(), i, threads_count);
            file_chunk.for_each_line(
                [&](std::string const & line) {
                    results[i].push_back(f(line));
                }
            );
        });
    }

    threads.run();
    mapped.close();

    results_t joined_results;
    for (auto const & v: results) {
        joined_results.insert(end(joined_results), std::begin(v), std::end(v));
    }
    return joined_results;
}

} //! data_structures
} //! paal
#endif // PAAL_MAPPED_FILE_HPP
