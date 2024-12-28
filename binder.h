#ifndef BINDER_H
#define BINDER_H

#include <algorithm>
#include <list>
#include <memory>
#include <stdexcept>
#include <map>
#include "binderT.h"
#include <string>
#include <type_traits>

// TODO copy-constructor and assignment operator should copy contents in case of
// shared V non-const reference by read()

// TODO maybe rollback ensure_unique()

// TODO write tests for copy-on-write

// TODO write tests for exception safety

namespace cxx {

template <typename K, typename V> class binder {

  private:
    using pair_type = std::pair<K, V>;
    using list_type = std::list<pair_type>;
    std::shared_ptr<list_type> notes_;
    std::shared_ptr<std::map<K, typename list_type::iterator>> index_;
    void ensure_unique();

  public:
  // DEBUG
    void copyFrom(binder<Random, V> const& other);


    binder();
    binder(binder const&);
    binder(binder&&) noexcept;
    ~binder() noexcept = default;

    binder& operator=(binder);

    void insert_front(K const& k, V const& v);
    void insert_after(K const& prev_k, K const& k, V const& v);
    void remove();
    void remove(K const&);
    V& read(K const&);
    V const& read(K const&) const;
    size_t size() const noexcept;
    void clear() noexcept;

    class const_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = V;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        const_iterator() : it_() {}

        const_iterator(typename list_type::const_iterator it) : it_(it) {}

        reference operator*() const { return it_->second; }
        pointer operator->() const { return &(it_->second); }

        const_iterator& operator++() {
            ++it_;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++it_;
            return tmp;
        }

        friend bool operator==(const const_iterator& a,
                               const const_iterator& b) {
            return a.it_ == b.it_;
        }
        friend bool operator!=(const const_iterator& a,
                               const const_iterator& b) {
            return a.it_ != b.it_;
        }

      private:
        typename list_type::const_iterator it_;
    };

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
};

template <typename K, typename V>
binder<K, V>::binder()
    : notes_(std::make_shared<list_type>()),
      index_(std::make_shared<std::map<K, typename list_type::iterator>>()) {}

template <typename K, typename V>
binder<K, V>::binder(binder const& other)
    : notes_(other.notes_), index_(other.index_) {}

template <typename K, typename V>
binder<K, V>::binder(binder&& other) noexcept {
    notes_ = other.notes_;
    index_ = other.index_;
    other.notes_ = nullptr;
    other.index_ = nullptr;
}

template <typename K, typename V>
binder<K, V>& binder<K, V>::operator=(binder other) {
    notes_ = other.notes_;
    index_ = other.index_;
    return *this;
}

template <typename K, typename V>
void binder<K, V>::insert_front(K const& k, V const& v) {
    if (index_->count(k)) {
        throw std::invalid_argument("Key already exists");
    }
    ensure_unique();
    notes_->emplace_front(k, v);
    try {
        index_->emplace(k, notes_->begin());
    } catch (...) {
        notes_->pop_front();
        throw;
    }   
}

template <typename K, typename V>
void binder<K, V>::insert_after(K const& prev_k, K const& k, V const& v) {
    auto it = index_->find(prev_k);
    if (it == index_->end() || index_->count(k)) {
        throw std::invalid_argument("Invalid key");
    }
    ensure_unique();
    auto new_it = notes_->emplace(std::next(it->second), k, v);
    try {
        index_->emplace(k, new_it);
    } catch (...) {
        notes_->erase(new_it);
        throw;
    }
}

template <typename K, typename V> void binder<K, V>::remove() {
    if (notes_->empty()) {
        throw std::invalid_argument("Binder is empty");
    }
    ensure_unique();
    index_->erase(notes_->begin()->first);
    notes_->pop_front();
}

template <typename K, typename V> void binder<K, V>::remove(K const& k) {
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }
    ensure_unique();
    notes_->erase(it->second);
    index_->erase(it);
}

template <typename K, typename V> V& binder<K, V>::read(K const& k) {
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }
    return it->second->second;
}

template <typename K, typename V>
V const& binder<K, V>::read(K const& k) const {
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }
    return it->second->second;
}

template <typename K, typename V> size_t binder<K, V>::size() const noexcept {
    return notes_->size();
}

template <typename K, typename V> void binder<K, V>::clear() noexcept {
    if (!notes_->empty()) {
        ensure_unique();
        notes_->clear();
        index_->clear();
    }
}

template <typename K, typename V>
typename binder<K, V>::const_iterator binder<K, V>::cbegin() const noexcept {
    return const_iterator(notes_->cbegin());
}

template <typename K, typename V>
typename binder<K, V>::const_iterator binder<K, V>::cend() const noexcept {
    return const_iterator(notes_->cend());
}

template <typename K, typename V> void binder<K, V>::ensure_unique() {
    if (!notes_.unique()) {
        auto notes_cpy = std::make_shared<list_type>(*notes_);
        auto index_cpy = std::make_shared<std::map<K, typename list_type::iterator>>();
        for (auto it = notes_cpy->begin(); it != notes_cpy->end(); it++) {
            index_cpy->emplace(it->first, it);
        }
        notes_ = notes_cpy;
        index_ = index_cpy;
    }
}

template <typename K, typename V>
void binder<K, V>::copyFrom(binder<Random, V> const& other) {
// use secret constructor

        list_type noteCpy;
        for (auto e: *(other.notes_)) {
            noteCpy.emplace_back(std::make_pair(Random((e.first).value, false), e.second));
        }
        auto index_cpy = std::make_shared<std::map<K, typename list_type::iterator>>();
        for (auto it = notes_->begin(); it != notes_->end(); it++) {
            index_cpy->emplace(it->first, it);
        }
        notes_ = std::make_shared<list_type>(&noteCpy);
        index_ = std::move(index_cpy);
}

} // namespace cxx

#endif // BINDER_H
