#ifndef BINDER_H
#define BINDER_H

#include <algorithm>
#include <list>
#include <memory>
#include <stdexcept>
#include <unordered_map>

// TODO are const everywhere where they should be?

// TODO are noexcept everywhere where it should be?

// TODO remove() should be faster (dont use find_if)

// TODO (task statement) why copying would be O(n log n) instead of O(n)?

// TODO remove unnecessary stuff from const_iterator nested class
// TODO operator++(int) ?

// TODO swaps in operator= ?

// TODO implement copy-on-write

// TODO do something with ensure_unique (probably it is some bullshit)

namespace cxx {

template <typename K, typename V> class binder {
  public:
    binder() noexcept;
    binder(binder const&);
    binder(binder&&) noexcept;

    binder& operator=(binder) noexcept;

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

        const_iterator(typename std::list<V>::const_iterator it) : it_(it) {}

        reference operator*() const { return *it_; }
        pointer operator->() const { return &(*it_); }

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
        typename std::list<V>::const_iterator it_;
    };

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

  private:
    std::shared_ptr<std::list<V>> notes_;
    std::shared_ptr<std::unordered_map<K, typename std::list<V>::iterator>>
        index_;
    void ensure_unique();
};

template <typename K, typename V>
binder<K, V>::binder() noexcept
    : notes_(std::make_shared<std::list<V>>()),
      index_(std::make_shared<
             std::unordered_map<K, typename std::list<V>::iterator>>()) {}

template <typename K, typename V>
binder<K, V>::binder(binder const& other)
    : notes_(other.notes_), index_(other.index_) {}

template <typename K, typename V>
binder<K, V>::binder(binder&& other) noexcept
    : notes_(std::move(other.notes_)), index_(std::move(other.index_)) {}

template <typename K, typename V>
binder<K, V>& binder<K, V>::operator=(binder other) noexcept {
    std::swap(notes_, other.notes_);
    std::swap(index_, other.index_);
    return *this;
}

template <typename K, typename V>
void binder<K, V>::insert_front(K const& k, V const& v) {
    ensure_unique();
    if (index_->count(k)) {
        throw std::invalid_argument("Key already exists");
    }
    notes_->emplace_front(v);
    index_->emplace(k, notes_->begin());
}

template <typename K, typename V>
void binder<K, V>::insert_after(K const& prev_k, K const& k, V const& v) {
    ensure_unique();
    auto it = index_->find(prev_k);
    if (it == index_->end() || index_->count(k)) {
        throw std::invalid_argument("Invalid key");
    }
    auto new_it = notes_->emplace(std::next(it->second), v);
    index_->emplace(k, new_it);
}

template <typename K, typename V> void binder<K, V>::remove() {
    ensure_unique();
    if (notes_->empty()) {
        throw std::invalid_argument("Binder is empty");
    }
    index_->erase(
        std::find_if(index_->begin(), index_->end(), [this](const auto& pair) {
            return pair.second == notes_->begin();
        }));
    notes_->pop_front();
}

template <typename K, typename V> void binder<K, V>::remove(K const& k) {
    ensure_unique();
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }
    notes_->erase(it->second);
    index_->erase(it);
}

template <typename K, typename V> V& binder<K, V>::read(K const& k) {
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }
    ensure_unique();
    return *(it->second);
}

template <typename K, typename V>
V const& binder<K, V>::read(K const& k) const {
    auto it = index_->find(k);
    if (it == index_->end()) {
        throw std::invalid_argument("Key not found");
    }
    return *(it->second);
}

template <typename K, typename V> size_t binder<K, V>::size() const noexcept {
    return notes_->size();
}

template <typename K, typename V> void binder<K, V>::clear() noexcept {
    ensure_unique();
    notes_->clear();
    index_->clear();
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
    if (!notes_.unique() || !index_.unique()) {
        notes_ = std::make_shared<std::list<V>>(*notes_);
        index_ = std::make_shared<
            std::unordered_map<K, typename std::list<V>::iterator>>(*index_);
    }
}

} // namespace cxx

#endif // BINDER_H