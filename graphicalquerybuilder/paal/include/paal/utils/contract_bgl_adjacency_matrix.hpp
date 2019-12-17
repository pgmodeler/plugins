//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file contract_bgl_adjacency_matrix.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-02-22
 */
#ifndef PAAL_CONTRACT_BGL_ADJACENCY_MATRIX_HPP
#define PAAL_CONTRACT_BGL_ADJACENCY_MATRIX_HPP

#include "paal/utils/type_functions.hpp"
#include "paal/utils/assign_updates.hpp"

#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/range/as_array.hpp>

namespace paal {
namespace utils {

/// contracts to vertices in adjacency_matrix
template <typename Idx, typename... GraphArgs>
void contract(boost::adjacency_matrix<GraphArgs...> &amatrix,
              Idx v, Idx w) {
    typedef boost::graph_traits<puretype(amatrix)> mtraits;
    typedef typename mtraits::edge_descriptor MEdge;
    auto const &weight_map = get(boost::edge_weight, amatrix);
    weight_map[edge(v, w, amatrix).first] = 0;
    for (auto && e : boost::as_array(out_edges(v, amatrix))) {
        MEdge f = edge(w, target(e, amatrix), amatrix).first;
        auto &we = weight_map[e];
        auto &wf = weight_map[f];
        wf = we = std::min(we, wf);

        // TODO hide  checking
        auto teste = edge(target(e, amatrix), w, amatrix).first;
        auto testf = edge(target(e, amatrix), v, amatrix).first;
        auto wte = weight_map[teste];
        auto wtf = weight_map[testf];
        assert(wte == wtf && wte == we);
    }
}
}
}
#endif // PAAL_CONTRACT_BGL_ADJACENCY_MATRIX_HPP
