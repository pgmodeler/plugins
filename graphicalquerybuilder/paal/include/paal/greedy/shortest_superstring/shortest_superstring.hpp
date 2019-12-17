//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file shortest_superstring.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-08-29
 */
#ifndef PAAL_SHORTEST_SUPERSTRING_HPP
#define PAAL_SHORTEST_SUPERSTRING_HPP


#include "paal/utils/algorithms/suffix_array/lcp.hpp"
#include "paal/utils/algorithms/suffix_array/suffix_array.hpp"
#include "paal/utils/type_functions.hpp"
#include "paal/greedy/shortest_superstring/prefix_tree.hpp"
#include "paal/data_structures/bimap.hpp"
#include "paal/utils/irange.hpp"

#include <boost/range/adaptors.hpp>

#include <vector>
#include <algorithm>
#include <utility>
#include <type_traits>

namespace paal {
namespace greedy {
namespace detail {

/**
 * @class shortest_superstring
 * @brief class to solve shortest superstring 3.5 aproximation,
 * using greedy algorithm:
 * contract pair of words with largest overlap until one word stays
    \snippet shortest_superstring_example.cpp Shortest Superstring Example
 *
 * example file is shortest_superstring_example.cpp
 *
 * @tparam Words
 */
template <typename Words> class shortest_superstring {
  public:
    typedef range_to_elem_t<Words> Word;
    typedef range_to_elem_t<Word> Letter;

    shortest_superstring(const Words &words)
        : m_length(count_sum_lenght(words)),
          m_prefix_tree(m_length, m_suffix_array, m_sum_words, m_lcp,
                        m_length_suffix_word) {

        initialize(words);

        suffix_array<Letter>(m_sum_words, m_suffix_array);

        data_structures::rank(m_suffix_array, m_rank);

        lcp(m_suffix_array, m_rank, m_lcp, m_sum_words);

        m_prefix_tree.build_prefix_tree();

        m_prefix_tree.fill_suffix_to_tree();

        join_all_words();
    }

    /**
     * @brief return word contains all words as subwords,
     * of lenght at most 3.5 larger than shortest superstring.
     *
        \snippet shortest_superstring_example.cpp Shortest Superstring Example
     *
     * example file is shortest_superstring_example.cpp
     *
     */
    Word get_solution() {
        Word answer;
        for (auto posInSumWords : irange(1, m_length)) {
            if ((!m_is_joined_sufiix[m_pos_to_word[posInSumWords]]) &&
                (m_sum_words[posInSumWords - 1] == m_prefix_tree.DELIMITER)) {
                for (int nextLetter = posInSumWords;
                     m_sum_words[nextLetter] != m_prefix_tree.DELIMITER;) {
                    answer.push_back(m_sum_words[nextLetter]);
                    if (m_res[nextLetter] == NO_OVERLAP_STARTS_HERE) {
                        ++nextLetter;
                    } else {
                        nextLetter = m_res[nextLetter];
                    }
                }
            }
        }
        return answer;
    }

  private:
    int count_sum_lenght(const Words &words) {
        int length = 1;
        for (auto const &word : words) {
            length += word.size() + 1;
        }
        return length;
    }

    void initialize(const Words &words) {
        m_nu_words = words.size();
        m_first_word_in_block_to_last_word_in_block.resize(m_length);
        m_last_word_in_block_to_first_word_in_block.resize(m_length);
        m_pos_to_word.resize(m_length);
        m_length_suffix_word.resize(m_length);

        m_suffix_array.resize(m_length);
        m_lcp.resize(m_length);
        m_rank.resize(m_length);
        m_res.resize(m_length);
        m_sum_words.resize(m_length);
        m_is_joined_prefix.resize(m_nu_words);
        m_is_joined_sufiix.resize(m_nu_words);
        m_length_to_pos.resize(m_length);

        m_length = 1;
        int wordsId = 0;
        for (auto const &word : words) {
            auto wordSize = boost::distance(word);
            m_length_words.push_back(wordSize);
            m_length_to_pos[wordSize].push_back(m_length);
            int noLetterInWord = 0;
            for (auto letter : word) {
                assert(letter != 0);
                auto globalLetterId = m_length + noLetterInWord;
                m_sum_words[globalLetterId] = letter;
                m_pos_to_word[globalLetterId] = wordsId;
                m_length_suffix_word[globalLetterId] =
                    wordSize - noLetterInWord;
                ++noLetterInWord;
            }
            m_first_word_in_block_to_last_word_in_block[m_length] = m_length;
            m_last_word_in_block_to_first_word_in_block[m_length] = m_length;
            m_length += wordSize + 1;
            ++wordsId;
        }
    }

    void erase_word_form_prefix_tree(int word) {
        m_is_joined_sufiix[m_pos_to_word[word]] = JOINED;
        m_prefix_tree.erase_word_form_prefix_tree(word);
    }

    void join_all_words() {
        auto ovelapSizeRange = irange(m_length) | boost::adaptors::reversed;
        for (auto overlapSize : ovelapSizeRange) {
            for (auto word : m_length_to_pos[overlapSize]) {
                if (m_lcp[m_rank[word]] >= overlapSize) { // check if word is
                                                          // substring
                    erase_word_form_prefix_tree(word);
                }
            }
        }

        // in each iteration we join all pair of words who have overlap size
        // equal overlapSize
        for (auto overlapSize : ovelapSizeRange) {
            for (auto word : m_long_words) {
                join_word(word, overlapSize);
            }
            for (auto word : m_length_to_pos[overlapSize]) {
                if (m_lcp[m_rank[word]] < overlapSize) { // check if word is not
                                                         // substring
                    m_long_words.push_back(word);
                }
            }
        }
    }

    void join_word(int ps, int overlap) {
        if (m_is_joined_prefix[m_pos_to_word[ps]] == JOINED) {
            return;
        };

        int suffix = m_rank[ps + m_length_words[m_pos_to_word[ps]] - overlap];

        int prefix = m_prefix_tree.get_prefixequal_to_suffix(
            suffix, m_last_word_in_block_to_first_word_in_block[ps]);

        if (prefix == NOT_PREFIX) {
            return;
        }
        m_res[ps + m_length_words[m_pos_to_word[ps]] - overlap - 1] = prefix;
        m_is_joined_prefix[m_pos_to_word[ps]] = JOINED;

        m_last_word_in_block_to_first_word_in_block[
            m_first_word_in_block_to_last_word_in_block[prefix]] =
            m_last_word_in_block_to_first_word_in_block[ps];
        m_first_word_in_block_to_last_word_in_block[
            m_last_word_in_block_to_first_word_in_block[prefix]] = prefix;
        erase_word_form_prefix_tree(prefix);
    }

    int m_length, m_nu_words;
    std::vector<Letter> m_sum_words;
    std::vector<int> m_first_word_in_block_to_last_word_in_block,
        m_last_word_in_block_to_first_word_in_block, m_pos_to_word,
        m_length_words, m_length_suffix_word, m_suffix_array, m_lcp, m_rank,
        m_res, m_long_words;
    std::vector<bool> m_is_joined_prefix, m_is_joined_sufiix;
    std::vector<std::vector<int>> m_length_to_pos;

    prefix_tree<Letter> m_prefix_tree;

    const static bool JOINED = true;

    const static int NO_OVERLAP_STARTS_HERE = 0;
    const static int NOT_PREFIX = -1;
};
} //!detail

/**
 * @param words
 * @brief return word contains all words as subwords,
 * of lenght at most 3.5 larger than shortest superstring.
 * words canot contains letter 0
    \snippet shortest_superstring_example.cpp Shortest Superstring Example
 *
 * example file is shortest_superstring_example.cpp
 * @tparam Words
 */
template <typename Words>
auto shortestSuperstring(const Words &words)->decltype(
    std::declval<detail::shortest_superstring<Words>>().get_solution()) {
    detail::shortest_superstring<Words> solver(words);
    return solver.get_solution();
}
;

}      //!greedy
}      //!paal
#endif // PAAL_SHORTEST_SUPERSTRING_HPP
