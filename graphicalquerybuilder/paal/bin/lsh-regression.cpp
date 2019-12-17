//=======================================================================
// Copyright (c) 2014 Karol Wegrzycki, Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lsh-regression.cpp
 * @brief lsh_regression binnary
 * @author Karol Wegrzycki, Andrzej Pacuk, Piotr Wygocki
 * @version 1.0
 * @date 2014-11-19
 */

#include "paal/data_structures/mapped_file.hpp"
#include "paal/regression/lsh_nearest_neighbors_regression.hpp"
#include "paal/utils/functors.hpp"
#include "paal/utils/performance_measures.hpp"
#include "paal/utils/read_svm.hpp"
#include "paal/utils/singleton_iterator.hpp"
#include "paal/utils/system_message.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/fill.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/size.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

using point_type_sparse = boost::numeric::ublas::compressed_vector<double>;
using point_type_dense =  boost::numeric::ublas::vector<double>;

using paal::utils::tuple_get;
namespace utils = paal::utils;
namespace po = boost::program_options;

enum Metric {HAMMING, L1, L2, JACCARD};
enum Vector_type {SPARSE, DENSE};

struct l1_tag{};
struct l2_tag{};
struct ham_tag{};
struct jaccard_tag{};

std::istream& operator>>(std::istream& in, Metric& metr) {
    std::string token;
    in >> token;
    boost::algorithm::to_lower(token);
    if (token == "hamming" || token == "h")
        metr = HAMMING;
    else if (token == "l1")
        metr = L1;
    else if (token == "l2")
        metr = L2;
    else if (token == "jaccard")
        metr = JACCARD;
    else
        assert(0 && "couldn't conclude metric name");
    return in;
}

std::istream& operator>>(std::istream& in, Vector_type & vec_type) {
    std::string token;
    in >> token;
    boost::algorithm::to_lower(token);
    if (token == "dense")
        vec_type = DENSE;
    else if (token == "sparse")
        vec_type = SPARSE;
    else
        assert(0 && "couldn't conclude sparse/dense representation");
    return in;
}

struct params {
    unsigned m_passes;
    unsigned m_nthread;
    std::size_t m_dimensions;
    std::size_t m_row_buffer_size;
    unsigned m_precision;
    int m_seed;
    double m_w;
    Vector_type m_dense;
    Metric m_metric;
};

auto get_function_generator(l1_tag, params const &p) {
    return paal::lsh::l_1_hash_function_generator<>{p.m_dimensions, p.m_w, std::default_random_engine(p.m_seed)};
}

auto get_function_generator(l2_tag, params const &p) {
    return paal::lsh::l_2_hash_function_generator<>{p.m_dimensions, p.m_w, std::default_random_engine(p.m_seed)};
}

auto get_function_generator(ham_tag, params const &p) {
    return paal::lsh::hamming_hash_function_generator(p.m_dimensions, std::default_random_engine(p.m_seed));
}

auto get_function_generator(jaccard_tag, params const &p) {
    return paal::lsh::jaccard_hash_function_generator(p.m_dimensions, std::default_random_engine(p.m_seed));
}

template <typename Row = point_type_sparse, typename LshFunctionTag>
void m_main(po::variables_map const &vm,
            params const &p,
            LshFunctionTag tag) {
    using lsh_fun = paal::pure_result_of_t<
                        paal::hash_function_tuple_generator<
                                    decltype(get_function_generator(tag, p))
                                                           >()
                                          >;
    using hash_result = typename std::remove_reference<
        typename std::result_of<lsh_fun(Row)>::type
        >::type;
    using model_t = paal::lsh_nearest_neighbors_regression<hash_result, lsh_fun>;
    using point_with_result_t = std::tuple<Row, int>;
    using boost::adaptors::transformed;
    constexpr paal::utils::tuple_get<0> get_coordinates{};
    constexpr paal::utils::tuple_get<1> get_result{};
    std::vector<point_with_result_t> points_buffer;

    model_t model;

    if (vm.count("model_in")) {
        Metric metric;
        std::size_t dimensions;
        std::ifstream ifs(vm["model_in"].as<std::string>());
        boost::archive::binary_iarchive ia(ifs);
        ia >> metric;
        ia >> dimensions;
        ia >> model;
        assert(metric == p.m_metric);
        assert(dimensions == p.m_dimensions);
    } else {
        model = paal::make_lsh_nearest_neighbors_regression_tuple_hash(
                        points_buffer | transformed(get_coordinates),
                        points_buffer | transformed(get_result),
                        p.m_passes,
                        get_function_generator(tag, p) , p.m_precision, p.m_nthread);
    }

    auto ignore_bad_row = [&](std::string const &bad_line) {
        utils::warning("following line will be ignored cause of bad format (typically more columns than passed dimensions): ", bad_line);
        return true;
    };

    if (vm.count("training_file")) {
        std::ifstream training_file_stream(vm["training_file"].as<std::string>());
        if (!training_file_stream.good()) {
            utils::failure("training file does not exist or is empty!");
        }

        points_buffer.reserve(p.m_row_buffer_size);
        while (training_file_stream.good()) {
            points_buffer.clear();
            auto dimensions = p.m_dimensions;
            paal::read_svm(training_file_stream, dimensions, points_buffer, p.m_row_buffer_size, ignore_bad_row);

            model.update(points_buffer | transformed(get_coordinates),
                         points_buffer | transformed(get_result),
                         p.m_nthread);
        }
    }

    if (vm.count("test_file")) {
        // Our algorithm returns range<tuple<Prediction,RealValue>>
        constexpr paal::utils::tuple_get<0> get_prediction{};
        constexpr paal::utils::tuple_get<1> get_real_value{};
        std::string const test_path = vm["test_file"].as<std::string>();

        // Lambda - for given line return tuple<LshPrediction,RealValue>>
        auto line_tester = [&](std::string const & line) {
            double test_result{};

            std::stringstream row_stream(line);
            using result_t = int;
            paal::detail::svm_row<Row, result_t> row{p.m_dimensions};
            if (!(row_stream >> row)) {
                ignore_bad_row(line);
                // prediction equal = result, should have the same impact on global result as ignoring the row
                return std::make_tuple(test_result, static_cast<result_t>(test_result));
            }

            auto test_point_iterator = paal::utils::make_singleton_range(row.get_coordinates());
            auto output_iterator = boost::make_function_output_iterator([&](double x){test_result = x;});

            model.test(test_point_iterator, output_iterator);

            return std::make_tuple(test_result,row.get_result());
        };

        // compute results using n threads and our lambda
        auto test_results = paal::data_structures::for_each_line(line_tester, test_path, p.m_nthread);

        auto loss = paal::log_loss<double>(test_results | transformed(get_prediction),
                                           test_results | transformed(get_real_value));

        utils::info("logloss on test set = ", loss, ", likelihood = ", paal::likelihood_from_log_loss(loss));

        if (vm.count("result_file") > 0) {
            std::ofstream result_file(vm["result_file"].as<std::string>());

            for (auto d: test_results | transformed(get_prediction)) {
                result_file << d << "\n";
            }
        }
    }

    if (vm.count("model_out")) {
        std::ofstream ofs(vm["model_out"].as<std::string>());
        boost::archive::binary_oarchive oa(ofs);
        oa << p.m_metric;
        oa << p.m_dimensions;
        oa << model;
    }
}

template <typename LshFunctionTag>
void choose_vector_type_main(po::variables_map const &vm,
            params const &p,
            LshFunctionTag tag) {
    if (p.m_dense == DENSE) {
        m_main<point_type_dense>(vm,p,tag);
    } else if( p.m_dense == SPARSE) {
        m_main<point_type_sparse>(vm,p,tag);
    }
}

void choose_vector_type_main(po::variables_map const &vm,
            params const &p,
            jaccard_tag tag) {
    assert(p.m_dense == SPARSE && "Jaccard metric supports only sparse rows representation.");
    m_main<point_type_sparse>(vm,p,tag);
}

int main(int argc, char** argv)
{
    params p{};

    po::options_description desc("LSH nearest neighbors regression - \n"\
            "suite for fast machine learning KNN algorithm which is using "\
            "locality sensitive hashing functions\n\nUsage:\n"\
            "This command will train on training file and output the predictions in test_file:\n"
            "\tlsh-regression --training_file path_to_training_file --test_file path_to_test_file --dimensions number_of_dimensions\n\n"\
            "If you want to use L1 metric, with 7 passes and 10 threads, and save model\n"\
            "you can use following command:\n"
            "\tlsh-regression -d training.svm -i 7 -n 10 -m L1 --dimensions number_of_dimensions --model_out model.lsh\n\n"\
            "Then if you want to use this model to make a prediction to result_file:\n"\
            "\tlsh-regression -t test.svm --model_in model.lsh -o results.txt\n\n"
            "Options description");

    desc.add_options()
        ("help,h", "help message")
        ("training_file,d", po::value<std::string>(), "training file path (in SVM format)")
        ("test_file,t", po::value<std::string>(), "test file path (in SVM format, it doesn't matter what label says)")
        ("model_in", po::value<std::string>(), "path to model, before doing any training or testing")
        ("model_out", po::value<std::string>(), "Write the model to this file when everything is done")
        ("result_file,o", po::value<std::string>(), "path to the file with prediction " \
                  "(float for every test in test set)")
        ("dimensions", po::value<std::size_t>(&p.m_dimensions), "number of dimensions")
        ("passes,i", po::value<unsigned>(&p.m_passes)->default_value(3), "number of iteration (default value = 3)")
        ("nthread,n", po::value<unsigned>(&p.m_nthread)->default_value(std::thread::hardware_concurrency()),
                 "number of threads (default = number of cores)")
        ("dense", po::value<Vector_type>(&p.m_dense)->default_value(SPARSE), "Dense/Sparse - Allows to use dense\
                  array representation (It might significantly speed up program for dense data)")
        ("metric,m", po::value<Metric>(&p.m_metric)->default_value(HAMMING), "Metric used for determining " \
                 "similarity between objects - [HAMMING/L1/L2/JACCARD] (default = Hamming)")
        ("precision,b", po::value<unsigned>(&p.m_precision)->default_value(10), "Number " \
                 "of hashing function that are encoding the object")
        ("parm_w,w", po::value<double>(&p.m_w)->default_value(1000.), "Parameter w " \
                 "should be essentially bigger than radius of expected test point neighborhood")
        ("row_buffer_size", po::value<std::size_t>(&p.m_row_buffer_size)->default_value(100000),
                 "size of row buffer (default value = 100000)")
        ("seed", po::value<int>(&p.m_seed)->default_value(0), "Seed of random number generator, (default = random)")
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

    if (vm.count("training_file") == 0 && vm.count("test_file") == 0) {
        error_with_usage("Neither training_file nor test_file were set");
    }

    if (vm.count("dimensions") == 0 && vm.count("model_in") == 0) {
        error_with_usage("Parameter dimensions was not set");
    }

    if (vm.count("training_file") == 0 && vm.count("model_in") == 0) {
        error_with_usage("If you don't set training file (training_file) you have to set input model (model_in)");
    }

    if (vm.count("model_in")) {
        Metric m;
        std::size_t dimensions;
        std::ifstream ifs(vm["model_in"].as<std::string>());
        boost::archive::binary_iarchive ia(ifs);

        ia >> m;
        ia >> dimensions;

        auto ignore_serializable_param = [&](std::string const &param_name, bool param_is_equal_to_serialized) {
            if (param_is_set_explicitly(param_name)) {
                if (param_is_equal_to_serialized) {
                    utils::warning("if input model is specified one does not have to specify param ", param_name);
                } else {
                    utils::warning("the specified param ", param_name, " is ignored, because it differs from the input model param ", param_name);
                }
            }
        };
        ignore_serializable_param("metric", m == p.m_metric);
        ignore_serializable_param("dimensions", dimensions == p.m_dimensions);

        auto ignored = [&](std::string const & param, std::string const & param_display) {
            if (param_is_set_explicitly(param)) {
                utils::warning("parameter ", param_display, " was set, but model_in is used, param ", param_display, " is discarded");
            }
        };
        ignored("parm_w", "w");
        ignored("precision", "precision");
        ignored("seed", "seed");
        ignored("passes", "passes");

        p.m_metric = m;
        p.m_dimensions = dimensions;
    } else {
        if (p.m_metric == HAMMING && param_is_set_explicitly("parm_w")) {
            utils::warning("parameter w was set, but hamming metric is used, param w is discarded");
        }
    }

    switch (p.m_metric) {
        case L1: choose_vector_type_main(vm, p, l1_tag{});
            break;

        case L2: choose_vector_type_main(vm, p, l2_tag{});
            break;

        case HAMMING: choose_vector_type_main(vm, p, ham_tag{});
            break;

        case JACCARD: choose_vector_type_main(vm, p, jaccard_tag{});
            break;

        default:
            assert(false);
    }
    return EXIT_SUCCESS;
}
