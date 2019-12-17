//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file splay_tree.hpp
 * @brief
 * @author unknown
 * @version 1.0
 * @date 2013-07-24
 */
#ifndef PAAL_SPLAY_TREE_HPP
#define PAAL_SPLAY_TREE_HPP

#include <boost/utility.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <unordered_map>

namespace paal {
namespace data_structures {

template <typename T> class splay_tree;

namespace detail {
/**
 * @param node root of a subtree
 * @returns size of subtree
  */
template <typename NPtr> std::size_t node_size(const NPtr &node) {
    return !node ? 0 : node->size();
}

template <typename V> class Node;

/**
 * @brief copy node pointer
 *
 * @tparam N
 * @param node
 *
 * @return
 */
template <typename V>
std::unique_ptr<Node<V>> copy_node(std::unique_ptr<Node<V>> const &node) {
    // TODO on c++14 change to make_unique
    return (!node) ? nullptr : std::unique_ptr<Node<V>>(new Node<V>(*node));
}

class forward_tag {};
class reversed_tag {};

inline reversed_tag other(forward_tag) {
    return reversed_tag{};
}

inline forward_tag other(reversed_tag) {
    return forward_tag{};
}


/**
 * Node of a splay_tree.
 *
 * Left/right relaxation should be understood as follows.
 * Meaning of left/right field changes iff xor of all bits on the path to the
 * root is 1. This enables us to reverse entire subtree in constant time (by
 * flipping bit in the root). Normalization is needed to determine which child
* is
 * the real left/right. */
template <typename V> class Node {
  public:
    using value_type = V;
    using node_type = Node<value_type>;
    using node_ptr = std::unique_ptr<node_type>;

    /** @param val stored value */
    explicit Node(const value_type &val)
        : val_(val), parent_(nullptr), reversed_(false), size_(1) {}

    /// constructor
    Node(const Node &n)
        : val_(n.val_), left_(copy_node(n.left_)), right_(copy_node(n.right_)),
          parent_(nullptr), reversed_(n.reversed_), size_(n.size_) {
        if (right_) {
            right_->parent_ = this;
        }
        if (left_) {
            left_->parent_ = this;
        }
    }

    /** @returns parent node */
    node_type *parent() { return parent_; }

    void set_parent(node_type *p) { parent_ = p; }

    /** @brief detaches this node from parent */
    void make_root() { parent_ = nullptr; }

    /** @returns left child pointer */
    node_ptr &left() {
        normalize();
        return left_;
    }

    /**
     * @brief sets left child pointer
     * @param node new child
      */
    void set_left(node_ptr node) {
        set(std::move(node), reversed_tag{});
    }

    /** @returns true right child pointer */
    node_ptr &right() {
        normalize();
        return right_;
    }

    void set_right(node_ptr node) {
        set(std::move(node), forward_tag{});
    }

    /**
     * @brief sets child pointer
     * @param node new child
      */
    template <typename Direction> void set(node_ptr node, Direction dir_tag) {
        normalize();
        set_internal(std::move(node), dir_tag);
        update_size();
    }

    /**
     * @brief sets child pointer (no relaxation)
     * @param node new child
      */
    template <typename Direction>
    void set_internal(node_ptr node, Direction dir_tag) {
        if (node != nullptr) {
            node->parent_ = this;
        }
        child(dir_tag) = std::move(node);
    }

    /** @brief recomputes subtree size from sizes of children's subtrees */
    void update_size() { size_ = 1 + node_size(left_) + node_size(right_); }

    node_ptr &child(reversed_tag) { return left(); }

    node_ptr &child(forward_tag) { return right(); }

    template <typename Direction> node_type *extreme(Direction dir_tag) {
        node_type *node = this;
        normalize();
        while (node->child(dir_tag).get() != nullptr) {
            node = node->child(dir_tag).get();
            node->normalize();
        }
        return node;
    }


    /** @returns next in same tree according to infix order
     * WARNING, we assume that path from root to the this node is normalized*/
    template <typename Direction> node_type *advance(Direction dir_tag) {
        node_type *node = child(dir_tag).get();
        if (node != nullptr) {
            return node->extreme(other(dir_tag));
        } else {
            node_type *last = nullptr;
            node = this;
            while (true) {
                last = node;
                node = node->parent();
                if (node == nullptr) {
                    return nullptr;
                } else if (node->child(other(dir_tag)).get() == last) {
                    return node;
                }
            }
        }
    }


    /** @returns next in same tree according to infix order
     * WARNING, we assume that path from root to the this node is normalized*/
    node_type *next() {
        return advance(forward_tag{});
    }

    /** @returns previous in same tree according to infix order */
    node_type *prev() {
        return advance(reversed_tag{});
    }

    /** @returns size of subtree */
    std::size_t size() { return size_; }

    /** @brief lazily reverses order in subtree */
    void subtree_reverse() { reversed_ ^= 1; }

    /** @brief locally relaxes tree */
    void normalize() {
        if (reversed_) {
            std::swap(left_, right_);
            if (left_ != nullptr) {
                left_->subtree_reverse();
            }
            if (right_ != nullptr) {
                right_->subtree_reverse();
            }
            reversed_ = false;
        }
    }

    /** @brief relaxes all nodes on path from root to this */
    void normalize_root_path() {
        node_type *node = parent();
        if (node != nullptr) {
            node->normalize_root_path();
        }
        normalize();
    }

    /// value of the node
    value_type val_;

  private:
    static const bool k_def_left = 0;
    node_ptr left_, right_;
    node_type *parent_;
    bool reversed_;
    std::size_t size_;
};


/**
 * splay_tree elements iterator.
 *
 * Traversing order is determined by template argument.
  */
template <typename V, typename direction_tag = forward_tag>
class Iterator
    : public boost::iterator_facade<Iterator<V, direction_tag>, Node<V> *,
                                    boost::bidirectional_traversal_tag, V &> {
    using ST = splay_tree<V>;
    using node_ptr = Node<V> *;

  public:
    using value_type = V;
    using node_type = Node<value_type>;

    /** @brief iterator after last element */
    Iterator() : current_(nullptr), rotation_cnt_(0), splay_(nullptr) {}

    /**
     * @brief iterator to element in given node
     * @param node node storing element pointed by iterator
     * @param splay pointer to the splay tree
      */
    explicit Iterator(node_ptr node, const ST *splay)
        : current_(node), rotation_cnt_(0), splay_(splay) {}

    /**
     * @brief copy constructor
     * @param other iterator to be copied
      */
    Iterator(const Iterator &other)
        : current_(other.current_), rotation_cnt_(0), splay_(other.splay_) {}

  private:
    friend class boost::iterator_core_access;
    friend class splay_tree<V>;

    void normalize() {
        if (rotation_cnt_ != splay_->get_rotation_cnt()) {
            current_->normalize_root_path();
            rotation_cnt_ = splay_->get_rotation_cnt();
        }
    }

    /** @brief increments iterator */
    void increment() {
        normalize();
        current_ = current_->advance(direction_tag{});
    }

    /** @brief decrements iterator */
    void decrement() {
        normalize();
        current_ = current_->advance(other(direction_tag{}));
    }

    /**
     * @param other iterator to be compared with
     * @returns true iff iterators point to the same node
      */
    bool equal(const Iterator &other) const {
        return this->current_ == other.current_;
    }

    /** @returns reference to pointed element */
    value_type &dereference() const { return current_->val_; }

    /** pointed node */
    node_ptr current_;
    std::size_t rotation_cnt_;
    const ST *splay_;
};

} //!detail

/**
 * Splay trees with logarithmic reversing of any subsequence.
 *
 * All tree operations are amortized logarithmic time in size of tree,
 * each element is indexed by number of smaller elements than this element.
 * Note that lookups are also amortized logarithmic in size of tree. Order of
 * elements is induced from infix ordering of nodes storing these elements.
  */
template <typename T> class splay_tree {
    detail::forward_tag forward_tag;
    detail::reversed_tag reversed_tag;

  public:
    using value_type = T;
    using node_type = detail::Node<value_type>;
    using node_ptr = typename node_type::node_ptr;
    using iterator = detail::Iterator<value_type, detail::forward_tag>;
    using const_iterator = const iterator;
    using reverse_iterator = detail::Iterator<value_type, detail::reversed_tag>;
    using const_reverse_iterator = const reverse_iterator;

    splay_tree() = default;

    /**
     * @brief constructs tree from elements between two iterators
     * @param b iterator to first element
     * @param e iterator to element after last
      */
    template <typename I> splay_tree(const I b, const I e) {
        root_ = build_tree(b, e);
    }

    /// constructor
    splay_tree(splay_tree &&splay) = default;

    /// operator=
    splay_tree &operator=(splay_tree &&splay) = default;
    // splay.rotation_cnt_ is not  0 after this move but it doesn't matter;

    /// operator=
    splay_tree &operator=(splay_tree &splay) {
        if (&splay == this) return *this;
        splay_tree sp(splay);
        *this = std::move(sp);
        return *this;
    }

    /// constructor
    splay_tree(const splay_tree &splay) : root_(copy_node(splay.root_)) {
        auto i = begin();
        auto e = end();
        for (; i != e; ++i) {
            t_to_node_.insert(std::make_pair(*i, i.current_));
        }
    }

    /**
     * @brief creates tree from elements in std::vector
     * @param array vector container
      */
    template <typename A> explicit splay_tree(const A &array) {
        root_ = build_tree(std::begin(array), std::end(array));
    }

    /** @returns forward iterator to first element in container */
    iterator begin() const {
        return (root_ == nullptr)
                   ? end()
                   : iterator(root_->extreme(reversed_tag), this);
    }

    /** @returns forward iterator to element after last in container */
    iterator end() const { return iterator(); }

    /** @returns reverse iterator to last element in container */
    reverse_iterator rbegin() {
        return (root_ == nullptr)
                   ? rend()
                   : reverse_iterator(root_->extreme(forward_tag), this);
    }

    /** @returns reverse iterator to element before first in container */
    reverse_iterator rend() { return reverse_iterator(); }

    /** @returns number of elements in tree */
    std::size_t size() const { return (root_ == nullptr) ? 0 : root_->size(); }

    /** @returns true iff tree contains no elements */
    bool empty() { return (root_ == nullptr); }

    /** @param i index of referenced element */
    value_type &operator[](std::size_t i) const { return find(i)->val_; }

    /** @param t referenced element */
    std::size_t get_idx(const T &t) const {
        node_type *node = t_to_node_.at(t);
        if (node == nullptr) {
            return -1;
        }
        node->normalize_root_path();

        std::size_t i = node_size(node->left());
        while (node != root_.get()) {
            if (node->parent()->left().get() == node) {
                node = node->parent();
            } else {
                node = node->parent();
                i += node_size(node->left()) + 1;
            }
        }
        return i;
    }

    /**
     * @brief gets rotationCnt()
     *
     * @return
     */
    std::size_t get_rotation_cnt() const { return rotation_cnt_; }

    /**
     * @brief splays tree according to splay policy
     * @param i index of element to become root
      */
    iterator splay(std::size_t i) const {
        splay_internal(find(i));
        return iterator(root_.get(), this);
    }

    /**
     * @brief splits sequence, modified this contains elements {0, ..., i}
     * @param i index of last element of this after modification
     * @returns tree containing elements {i+1, ...}
      */
    splay_tree split_higher(std::size_t i) { return split(i, forward_tag); }

    /**
     * @brief splits sequence, modified this contains elements {i, ...}
     * @param i index of first element of this after modification
     * @returns tree containing elements {0, ..., i-1}
      */
    splay_tree split_lower(std::size_t i) { return split(i, reversed_tag); }

    /**
     * @brief merges given tree to the right of the biggest element of this
     * @param other tree to be merged
      */
    void merge_right(splay_tree other) { merge(std::move(other), forward_tag); }

    /**
     * @brief merges given tree to the left of the smallest element of this
     * @param other tree to be merged
      */
    void merge_left(splay_tree other) { merge(std::move(other), reversed_tag); }

    /**
     * @brief reverses subsequence of elements with indices in {i, ..., j}
     * @param i index of first element of subsequence
     * @param j index of last element of subsequence
      */
    void reverse(std::size_t i, std::size_t j) {
        assert(i <= j);
        // split lower
        splay_tree<value_type> ltree = split_lower(i);
        // split higher
        splay_tree<value_type> rtree = split_higher(j - i);
        // reverse
        root_->subtree_reverse();
        // merge
        merge_left(std::move(ltree));
        merge_right(std::move(rtree));
    }

  private:
    /** @brief creates tree with given node as a root */
    explicit splay_tree(node_ptr root) : root_(std::move(root)) {}

    template <typename Direction>
    splay_tree split(std::size_t i, Direction dir_tag) {
        splay(i);
        node_ptr new_root = std::move(root_->child(dir_tag));
        if (new_root != nullptr) {
            new_root->make_root();
        }
        if (root_ != nullptr) {
            root_->update_size();
        }

        return splay_tree<value_type>(std::move(new_root));
    }

    iterator splay(detail::forward_tag) const {
        return splay(root_->size() - 1);
    }

    iterator splay(detail::reversed_tag) const { return splay(0); }

    template <typename Direction>
    void merge(splay_tree other, Direction dir_tag) {
        if (other.root_ == nullptr) {
            return;
        }
        splay(dir_tag);
        assert(root_->child(dir_tag) == nullptr);
        root_->set(std::move(other.root_), dir_tag);
    }


    node_ptr &get_parent(node_ptr &node) const {
        assert(node);
        node_type *parent = node->parent();
        assert(parent != nullptr);
        node_type *granpa = parent->parent();
        if (granpa == nullptr) {
            return root_;
        } else {
            if (granpa->left().get() == parent) {
                return granpa->left();
            } else {
                assert(granpa->right().get() == parent);
                return granpa->right();
            }
        }
    }

    /**
     * @brief splays given node to tree root
     * @param node node of a tree to be moved to root
      */
    void splay_internal(node_ptr &node) const {
        assert(node);
        if (node == root_) {
            return;
        }
        node_ptr &parent = get_parent(node);
        if (parent == root_) {
            if (node == parent->left()) {
                rotate(root_, forward_tag);
            } else {
                assert(node == parent->right());
                rotate(root_, reversed_tag);
            }
        } else {
            node_ptr &grand = get_parent(parent);
            if (node == parent->left() && parent == grand->left()) {
                rotate(grand, forward_tag);
                rotate(grand, forward_tag);
            } else if (node == parent->right() && parent == grand->right()) {
                rotate(grand, reversed_tag);
                rotate(grand, reversed_tag);
            } else if (node == parent->right() && parent == grand->left()) {
                rotate(parent, reversed_tag);
                rotate(grand, forward_tag);
            } else if (node == parent->left() && parent == grand->right()) {
                rotate(parent, forward_tag);
                rotate(grand, reversed_tag);
            }
            splay_internal(grand);
        }
    }

    /**
     * @brief rotates tree over given node
     * @param parent pivot of rotation
      */
    template <typename Direction>
    void rotate(node_ptr &parent, Direction dir_tag) const {
        auto other_tag = other(dir_tag);
        node_ptr node = std::move(parent->child(other_tag));
        node.swap(parent);
        parent->set_parent(node->parent());
        node->set(std::move(parent->child(dir_tag)),
                  other_tag); // node size is updated here
        parent->set(std::move(node), dir_tag); // node size is updated here
    }


    /**
     * @brief recursively creates balanced tree from a structure described
     *        by two random access iterators
     * @param b iterator to first element
     * @param e iterator to element after last
      */
    template <typename I> node_ptr build_tree(const I b, const I e) {
        if (b >= e) {
            return nullptr;
        }
        std::size_t m = (e - b) / 2;
        node_ptr node{ new node_type(*(b + m)) };
        bool ret =
            t_to_node_.insert(std::make_pair(*(b + m), node.get())).second;
        assert(ret);
        node->set_left(build_tree(b, b + m));
        node->set_right(build_tree(b + m + 1, e));
        return node;
    }


    /**
     * @brief find n-th element in tree (counting from zero)
     * @param i number of elements smaller than element to be returned
     * @returns pointer to found node or nullptr if doesn't exist
      */
    node_ptr &find(std::size_t i) const {
        node_ptr *node = &root_;
        while (true) {
            if (!*node) {
                return *node;
            }
            node_ptr *left = &((*node)->left());
            std::size_t left_size = node_size(*left);
            if (left_size == i) {
                return *node;
            } else if (left_size > i) {
                node = left;
            } else {
                i -= left_size + 1;
                node = &(*node)->right();
            }
        }
    }

    /** root node of a tree */
    std::size_t rotation_cnt_ = 0; // to keep iterators consistent with tree
    mutable node_ptr root_;
    std::unordered_map<T, node_type *> t_to_node_;
};
}
}

#endif // PAAL_SPLAY_TREE_HPP
