//=======================================================================
// Copyright (c) 2013 Piotr Smulewicz
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file prefix_tree.hpp
 * @brief
 * @author Piotr Smulewicz
 * @version 1.0
 * @date 2013-09-10
 */
#ifndef PAAL_PREFIX_TREE_HPP
#define PAAL_PREFIX_TREE_HPP

#include <vector>
namespace paal {
namespace greedy {
namespace detail {

template <typename Letter> class prefix_tree {

    struct Node {
        Letter letter = DELIMITER;
        Node *son = CHILDLESS;
        std::vector<int> prefixes; // ends of all prefixes of single words in
                                   // concatenate words corresponding to Node
        Node(int _letter) : letter(_letter) {};
        Node() {}; // root
    };

  public:
    prefix_tree(int length, std::vector<int> const &suffix_array,
                std::vector<Letter> const &sumWords,
                std::vector<int> const &lcp,
                std::vector<int> const &lengthSuffixWord)
        : m_length(length), m_prefix_tree(m_length), m_which_son_am_i(m_length),
          m_prefix_to_tree(m_length), m_suffix_to_tree(m_length),
          m_suffix_array(suffix_array), m_sum_words(sumWords), m_lcp(lcp),
          m_length_suffix_word(lengthSuffixWord) {}

    void build_prefix_tree() {
        m_prefix_tree.push_back(Node()); // root
        for (auto suffix : m_suffix_array) {
            // memory protection and we add only whole words in lexographic
            // order
            if ((suffix != 0) && (m_sum_words[suffix - 1] == DELIMITER)) {
                add_word_to_prefix_tree(suffix);
            }
        }
    }

    void erase_word_form_prefix_tree(int wordBegin) {
        for (int letterOfWord = 0;
             m_sum_words[letterOfWord + wordBegin] != DELIMITER;
             ++letterOfWord) {
            auto letterIdx = wordBegin + letterOfWord;
            auto whichSon = m_which_son_am_i[letterIdx];
            auto &nodePrefixes = m_prefix_to_tree[letterIdx]->prefixes;
            assert(std::size_t(whichSon) < nodePrefixes.size());
            int lastPrefix = nodePrefixes.back();
            nodePrefixes[whichSon] = lastPrefix;
            m_which_son_am_i[lastPrefix + letterOfWord] = whichSon;
            nodePrefixes.pop_back();
        }
    }

    // for all suffix of word: if suffix is equal to any prefix of word we
    // remember position in prefix tree coresponding to suffix
    void fill_suffix_to_tree() {
        for (int suffix = m_length - 1, lastWord = 0, commonPrefix = 0;
             suffix > 0; suffix--) {
            auto beginOfSuffix = m_suffix_array[suffix];
            if (beginOfSuffix == 0 ||
                m_sum_words[beginOfSuffix - 1] == DELIMITER) {
                lastWord = beginOfSuffix;
                commonPrefix = m_lcp[suffix];
            } else {
                if (commonPrefix == m_length_suffix_word[beginOfSuffix]) {
                    m_suffix_to_tree[suffix] =
                        m_prefix_to_tree[lastWord + commonPrefix - 1];
                }
                if (m_lcp[suffix] < commonPrefix) {
                    commonPrefix = m_lcp[suffix];
                }
            }
        }
    }

    int get_prefixequal_to_suffix(int suffix, int firstWordInBlock) {
        Node *nodeCorrespondingToSuffix = m_suffix_to_tree[suffix];
        if (nodeCorrespondingToSuffix == NO_SUFFIX_IN_TREE) {
            return NOT_PREFIX;
        }
        auto const &overlapPrefixes = nodeCorrespondingToSuffix->prefixes;

        if (overlapPrefixes.size()) {
            int whichPrefix = ANY_PREFIX; // which prefix of prefixes equal to
                                          // suffix, will be joined
            // check if first prefix belong to same block as prefix (avoid
            // loops)
            if (overlapPrefixes[whichPrefix] == firstWordInBlock) {
                if (overlapPrefixes.size() >= 2) {
                    whichPrefix = ANY_OTHER_PREFIX;
                } else {
                    return NOT_PREFIX;
                }
            }
            return overlapPrefixes[whichPrefix];
        } else {
            return NOT_PREFIX;
        }
    }

  private:
    void add_word_to_prefix_tree(int word) {
        Node *node = &m_prefix_tree[ROOT];
        int letter = word;
        // we go by patch until Letter on patch all equal to letter in words
        // we only check last son because we add words in lexographic order
        while (node->son != CHILDLESS &&
               node->son->letter == m_sum_words[letter] &&
               m_sum_words[letter] != DELIMITER) {
            node = node->son;
            ++letter;
        }
        // we add new Node
        while (m_sum_words[letter]) {
            // if this asserts, you have very strange implementation of stl
            assert(m_prefix_tree.capacity() > m_prefix_tree.size());
            m_prefix_tree.push_back(Node(m_sum_words[letter]));
            node->son = &m_prefix_tree.back();
            node = &m_prefix_tree.back();
            ++letter;
        }
        node = &m_prefix_tree[ROOT];
        letter = word;
        // we fill:
        //    m_prefix_to_tree
        //    m_which_son_am_i
        // and add to Node.prefixes coresponding prefixes
        while (m_sum_words[letter] != DELIMITER) {
            node = node->son;
            m_prefix_to_tree[letter] = node;
            m_which_son_am_i[letter] = node->prefixes.size();
            node->prefixes.push_back(word);
            ++letter;
        }
    }
    int m_length;

    std::vector<Node> m_prefix_tree;
    std::vector<int> m_which_son_am_i;

    std::vector<Node *> m_prefix_to_tree;
    std::vector<Node *> m_suffix_to_tree;

    const std::vector<int> &m_suffix_array;
    const std::vector<Letter> &m_sum_words;
    const std::vector<int> &m_lcp;
    const std::vector<int> &m_length_suffix_word;

    const static int ROOT = 0;
    const static int NOT_PREFIX = -1;
    const static int ANY_PREFIX = 0;
    const static int ANY_OTHER_PREFIX = 1;
    const static std::nullptr_t NO_SUFFIX_IN_TREE;
    const static std::nullptr_t CHILDLESS;

  public:
    const static Letter DELIMITER = 0;
};
}      //!detail
}      //!greedy
}      //!paal
#endif // PAAL_PREFIX_TREE_HPP
