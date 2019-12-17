/**
 * @file thread_pool.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2014-12-03
 */
#ifndef PAAL_THREAD_POOL_HPP
#define PAAL_THREAD_POOL_HPP

#define BOOST_ERROR_CODE_HEADER_ONLY
#define BOOST_SYSTEM_NO_DEPRECATED

#include <boost/asio/io_service.hpp>

#include <cassert>
#include <cstdlib>
#include <thread>
#include <utility>
#include <vector>

namespace paal {

///simple threadpool, class uses also current thread!
class thread_pool {
    boost::asio::io_service m_io_service;
    std::vector<std::thread> m_threadpool;
    std::size_t m_threads_besides_current;

public:
    ///constructor
    thread_pool(std::size_t size) : m_threads_besides_current(size - 1) {
        assert(size > 0);
        m_threadpool.reserve(m_threads_besides_current);
    }

    ///post new task
    template <typename Functor>
    void post(Functor f) {
        //TODO when there is only one thread in thread pool task could be run instantly
        m_io_service.post(std::move(f));
    }

    ///run all posted tasks (blocking)
    void run() {
        auto io_run = [&](){m_io_service.run();};
        for(std::size_t i = 0; i < m_threads_besides_current; ++i) {
            m_threadpool.emplace_back(io_run);
        }
        // if threads_count == 1, we run all tasks in current thread
        io_run();

        for (auto & thread : m_threadpool) thread.join();
    }
};


}//!paal

#endif /* PAAL_THREAD_POOL_HPP */
