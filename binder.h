#ifndef BINDER_H
#define BINDER_H

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>

namespace cxx {

template <typename K, typename V> class binder {

  private:
    using pair_type = std::pair<K, V>;
    using list_type = std::list<pair_type>;
    std::shared_ptr<list_type> notes_;
    std::shared_ptr<std::map<K, typename list_type::iterator>> index_;

    // True iff the binder has a shared writeable reference.
    bool writeable_ref;

    // It makes a deep copy of the original binder.
    void make_copy();

    // `make_copy` but in case of exception, restores the original state.
    void make_copy_safe();

  public:
    binder();

    // Copy constructor. If the original binder has
    // shared writeable reference, it makes a deep copy.
    binder(binder const&);

    binder(binder&&) noexcept;
    ~binder() noexcept = default;

    // Copy assignment operator. If the original binder has
    // shared writeable reference, it makes a deep copy.
    binder& operator=(binder const&);

    // These methods make a deep copy of the original binder if it is shared.
    void insert_front(K const& k, V const& v);
    void insert_after(K const& prev_k, K const& k, V const& v);
    void remove();
    void remove(K const&);
    V& read(K const&);
    V const& read(K const&) const;
    void clear();

    size_t size() const noexcept;

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

template <typename K, typename V> binder<K, V>::binder() {
    notes_ = std::make_shared<list_type>();
    index_ = std::make_shared<std::map<K, typename list_type::iterator>>();
    writeable_ref = false;
}

template <typename K, typename V> binder<K, V>::binder(binder const& other) {
    auto old_notes = notes_;
    auto old_index = index_;

    notes_ = other.notes_;
    index_ = other.index_;

    if (other.writeable_ref) {
        try {
            make_copy();
        } catch (...) {
            notes_ = old_notes;
            index_ = old_index;
            throw;
        }
    }

    writeable_ref = false;
}

template <typename K, typename V>
binder<K, V>::binder(binder&& other) noexcept {
    notes_ = other.notes_;
    index_ = other.index_;
    writeable_ref = other.writeable_ref;
    other.notes_ = nullptr;
    other.index_ = nullptr;
    other.writeable_ref = false;
}

template <typename K, typename V>
binder<K, V>& binder<K, V>::operator=(binder const& other) {
    if (this == &other) {
        return *this;
    }

    auto old_notes = notes_;
    auto old_index = index_;

    notes_ = other.notes_;
    index_ = other.index_;

    if (other.writeable_ref) {
        try {
            make_copy();
        } catch (...) {
            notes_ = old_notes;
            index_ = old_index;
            throw;
        }
    }

    writeable_ref = false;

    return *this;
}

template <typename K, typename V>
void binder<K, V>::insert_front(K const& k, V const& v) {
    if (index_->count(k)) {
        throw std::invalid_argument("Key already exists");
    }

    bool shared = !notes_.unique();

    if (shared) {
        auto old_notes = notes_;
        auto old_index = index_;
        try {
            make_copy();
            notes_->emplace_front(k, v);
            index_->emplace(k, notes_->begin());
        } catch (...) {
            notes_ = old_notes;
            index_ = old_index;
            throw;
        }
    } else {
        size_t prv_size = notes_->size();
        try {
            notes_->emplace_front(k, v);
            index_->emplace(k, notes_->begin());
        } catch (...) {
            if (notes_->size() != prv_size) {
                notes_->pop_front();
            }
            if (index_->size() != prv_size) {
                index_->erase(k);
            }
            throw;
        }
    }

    writeable_ref = false;
}

template <typename K, typename V>
void binder<K, V>::insert_after(K const& prev_k, K const& k, V const& v) {
    auto it = index_->find(prev_k);
    if (it == index_->end() || index_->count(k)) {
        throw std::invalid_argument("Invalid key");
    }

    bool shared = !notes_.unique();
    auto new_it = notes_->end();

    if (shared) {
        auto old_notes = notes_;
        auto old_index = index_;
        try {
            make_copy();
            new_it = notes_->emplace(std::next(it->second), k, v);
            index_->emplace(k, new_it);
        } catch (...) {
            notes_ = old_notes;
            index_ = old_index;
            throw;
        }
    } else {
        size_t prv_size = notes_->size();
        try {
            new_it = notes_->emplace(std::next(it->second), k, v);
            index_->emplace(k, new_it);
        } catch (...) {
            if (notes_->size() != prv_size) {
                notes_->erase(new_it);
            }
            if (index_->size() != prv_size) {
                index_->erase(k);
            }
            throw;
        }
    }

    writeable_ref = false;
}

template <typename K, typename V> void binder<K, V>::remove() {
    if (notes_->empty()) {
        throw std::invalid_argument("Binder is empty");
    }

    bool shared = !notes_.unique();

    if (shared) {
        make_copy_safe();
    }

    index_->erase(notes_->begin()->first);
    notes_->pop_front();

    writeable_ref = false;
}

template <typename K, typename V> void binder<K, V>::remove(K const& k) {
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }

    bool shared = !notes_.unique();

    if (shared) {
        make_copy_safe();
    }

    notes_->erase(it->second);
    index_->erase(it);

    writeable_ref = false;
}

template <typename K, typename V> V& binder<K, V>::read(K const& k) {
    if (index_->find(k) == index_->end()) {
        throw std::invalid_argument("Key not found");
    }

    bool shared = !notes_.unique();

    if (shared) {
        make_copy_safe();
    }

    writeable_ref = true;
    return index_->find(k)->second->second;
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

template <typename K, typename V> void binder<K, V>::clear() {
    if (notes_->empty()) {
        return;
    }

    bool shared = !notes_.unique();

    if (shared) {
        make_copy_safe();
    }

    notes_->clear();
    index_->clear();
    writeable_ref = false;
}

template <typename K, typename V>
typename binder<K, V>::const_iterator binder<K, V>::cbegin() const noexcept {
    return const_iterator(notes_->cbegin());
}

template <typename K, typename V>
typename binder<K, V>::const_iterator binder<K, V>::cend() const noexcept {
    return const_iterator(notes_->cend());
}

template <typename K, typename V> void binder<K, V>::make_copy() {
    notes_ = std::make_shared<list_type>(*notes_);
    index_ = std::make_shared<std::map<K, typename list_type::iterator>>();
    for (auto it = notes_->begin(); it != notes_->end(); it++) {
        index_->emplace(it->first, it);
    }
}

template <typename K, typename V> void binder<K, V>::make_copy_safe() {
    auto old_notes = notes_;
    auto old_index = index_;
    try {
        make_copy();
    } catch (...) {
        notes_ = old_notes;
        index_ = old_index;
        throw;
    }
}
} // namespace cxx

#endif // BINDER_H
