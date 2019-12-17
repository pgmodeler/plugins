//=======================================================================
// Copyright (c) 2015
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file frequent-directions.cpp
 * @brief frequent_directions binnary
 * @author Tomasz Strozak
 * @version 1.0
 * @date 2015-07-13
 */

#include "paal/sketch/frequent_directions.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/irange.hpp"
#include "paal/utils/print_collection.hpp"
#include "paal/utils/read_rows.hpp"
#include "paal/utils/system_message.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace utils = paal::utils;
namespace po = boost::program_options;
using coordinate_t = double;
using matrix_t = boost::numeric::ublas::matrix<coordinate_t>;
using fd_t = paal::frequent_directions<matrix_t>;

struct params {
    size_t m_sketch_rows;
    size_t m_sketch_compress_size;
    //TODO
    //unsigned m_nthread;
    size_t m_row_buffer_size;
    bool m_compress_at_end;
};

void m_main(po::variables_map const &vm, params const &p,
            std::istream &input_stream, std::ostream &output_stream) {
    fd_t fd_sketch;

    std::vector<std::vector<coordinate_t>> row_buffer;
    row_buffer.reserve(p.m_row_buffer_size);

    auto ignore_bad_row = [&](std::string const &bad_line) {
        utils::warning("following line will be ignored cause of bad format: ", bad_line);
        return true;
    };

    std::size_t rows_count;
    std::size_t columns_count;
    if (vm.count("model_in")) {
        std::ifstream ifs(vm["model_in"].as<std::string>());
        boost::archive::binary_iarchive ia(ifs);
        ia >> fd_sketch;
        auto sketch = fd_sketch.get_sketch().first;
        rows_count = sketch.size1();
        columns_count = sketch.size2();
    }
    else {
        paal::read_rows_first_row_size<coordinate_t>
            (input_stream, row_buffer, p.m_row_buffer_size, ignore_bad_row);

        if(row_buffer.empty()) {
            utils::failure("Empty input data");
        }

        rows_count = p.m_sketch_rows;
        columns_count = boost::size(row_buffer.front());
        if(vm.count("sketch_compress_size")) {
            fd_sketch = paal::make_frequent_directions<coordinate_t>(rows_count, columns_count, p.m_sketch_compress_size);
        }
        else {
            fd_sketch = paal::make_frequent_directions<coordinate_t>(rows_count, columns_count);
        }

        fd_sketch.update_range(row_buffer);
    }

    while (input_stream.good()) {
        row_buffer.clear();
        paal::read_rows<coordinate_t>
            (input_stream, row_buffer, columns_count, p.m_row_buffer_size, ignore_bad_row);
        fd_sketch.update_range(row_buffer);
    }

    if (p.m_compress_at_end) {
        fd_sketch.compress();
    }

    auto sketch = fd_sketch.get_sketch().first;
    boost::numeric::ublas::matrix_range<matrix_t> sketch_range (sketch,
         boost::numeric::ublas::range(0, fd_sketch.get_sketch().second),
         boost::numeric::ublas::range(0, columns_count));
    paal::print_matrix(output_stream, sketch_range, " ");
    output_stream << std::endl;

    if (vm.count("model_out")) {
        std::ofstream ofs(vm["model_out"].as<std::string>());
        boost::archive::binary_oarchive oa(ofs);
        oa << fd_sketch;
    }

}

int main(int argc, char** argv) {
    params p{};

    po::options_description desc("Frequent-directions - \n"\
            "suite for a matrix sketching using Singular Value Decomposition\n\nUsage:\n"\
            "This command will read data from standard input and write computed sketch to standard output:\n"\
            "\tfrequent-directions --sketch_rows numer_of_sketch_rows\n\n"\
            "If you want to read data from an input_file and write computed sketch to an output_file you can use following command:\n"\
            "\tfrequent-directions --input input_file --output output_file -r rows\n\n"\
            "If you want to change compress_size and save model you can use following command:\n"\
            "\tfrequent-directions -i input_file -r rows -s compress_size --model_out model\n\n"\
            "Then if you want to use this model and add additional data:\n"\
            "\tfrequent-directions -i input_file --model_in model\n\n"\
            "Options description");

    desc.add_options()
        ("help,h", "help message")
        ("input,i", po::value<std::string>(), "path to the file with input data in csv format with space as delimiter, "\
                "(default read from standart input)")
        ("output,o", po::value<std::string>(), "path to the file with result sketch matrix, only nonzero rows are printed, "\
                "(default write to standart output)")
        ("sketch_rows,r", po::value<std::size_t>(&p.m_sketch_rows), "number of sketch rows")
        ("sketch_compress_size,s", po::value<size_t>(&p.m_sketch_compress_size), "sketch compress size, "\
                "(default is half of number of sketch rows)")
        ("model_in", po::value<std::string>(), "read the sketch model from this file")
        ("model_out", po::value<std::string>(), "write the sketch model to this file")
        ("final_compress", po::value<bool>(&p.m_compress_at_end)->default_value(true),
                "determine if sketch will be compressed after update all data, "\
                "compression in the final phase is necessary to fulfill sketch approximation ratios")
    //TODO
    //    ("nthread,n", po::value<unsigned>(&p.m_nthread)->default_value(std::thread::hardware_concurrency()),
    //          "number of threads (default = number of cores)")
        ("row_buffer_size", po::value<std::size_t>(&p.m_row_buffer_size)->default_value(100000),
                  "size of row buffer (default value = 100000)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    auto param_is_set_explicitly = [&vm] (const std::string &param_name) {
        return vm.count(param_name) > 0 && !vm[param_name].defaulted();
    };

    if (vm.count("help")) {
        utils::info(desc);
        return EXIT_SUCCESS;
    }

    auto error_with_usage = [&] (const std::string &message) {
        utils::failure(message, "\n", desc);
    };

    if (vm.count("model_in") == 0 && vm.count("sketch_rows") == 0) {
        error_with_usage("Input model sketch or number of sketch rows was not set");
    }

    if (vm.count("model_in")) {
        auto ignored = [&](std::string const & param) {
            if (param_is_set_explicitly(param)) {
                utils::warning("parameter ", param, " was set, but model_in is used, param ", param, " is discarded");
            }
        };
        ignored("sketch_rows");
        ignored("sketch_compress_size");
    }

    if (p.m_row_buffer_size <= 0) {
        error_with_usage("Size of row buffer must be positive");
    }

    std::ifstream ifs;
    if (vm.count("input")) {
        ifs.open(vm["input"].as<std::string>());
    }

    std::ofstream ofs;
    if (vm.count("output")) {
        ofs.open(vm["output"].as<std::string>());
    }

    m_main(vm, p,
           vm.count("input") ? ifs : std::cin,
           vm.count("output") ? ofs : std::cout);


    return EXIT_SUCCESS;
}
