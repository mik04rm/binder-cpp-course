#include "binder.h"
#include <cassert>
#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <ctime>
#include <new>      // For std::bad_alloc
#include <cstdlib>  // For rand, srand
#include <iostream>
#include <map>

using std::invalid_argument;
using std::pair;
using std::make_pair;
using std::string;
using cxx::binder;

bool fail = false;

// Override the global new operator
void* operator new(std::size_t size) {
    // Simulate random failure
    static int fail_count = 0;
    if(fail)
        fail_count++;
    
    //std::cout << "count" << fail_count << "fail" << fail << "\n";

    if (fail && fail_count % 10 == 0) { // Fail every 5th allocation
        throw std::bad_alloc();
    }

    // Allocate memory
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc(); // Throw if allocation fails
    }
    return ptr;
}

// Override the global delete operator
void operator delete(void* ptr) noexcept {
    std::free(ptr);
}


template<typename T, typename V>
bool operator==(binder<T, V> const& b1, binder<T, V> const& b2) {
    if (b1.size() != b2.size())
        return false;
    try {
        auto it1 = b1.cbegin();
        auto it2 = b2.cbegin();
        for(int i = 0; i < b1.size(); i++)
            if (*it1 != *it2)
                return false;
            it1++; it2++;

    } catch(...) {
        throw std::runtime_error("hmmm");
        return false;
    }
    return true;
}

template <typename K, typename V>
void snapshot(binder<K, V>& me, binder<K, V> const& you) {
    bool failPrev = fail;
    fail = false;
    me.deepCopy(you);
    fail = failPrev;
}

/*****/
template <typename K, typename V>
void testDel(binder<K, V>& bin, std::map<K, V>& lookup) {
    if (lookup.size() == 0)
        return;
    fail = false;
    int pos = std::rand() % lookup.size();
    auto it = lookup.begin();
    binder<K, V> tester;
    snapshot(tester, bin);
    for(int i = 0; i<pos; i++)
        it++;
    K val1 = it->first;
    fail = true;
    try {
        bin.remove(val1);
        fail = false;
        lookup.erase(it);
        fail = true;
    } catch(...) {
        assert(bin == tester);
    }

}
template <typename K, typename V>
void testIns(binder<K, V>& bin, std::map<K, V>& lookup, K i1, V i2) {
    if (lookup.size() == 0)
        return;
    fail = false;
    int pos = std::rand() % lookup.size();
    auto it = lookup.begin();
    binder<K, V> tester;
    snapshot(tester, bin);
    for(int i = 0; i<pos; i++)
        it++;
    K val1 = it->first;
    fail = true;
    try {
        bin.insert_after(val1, i1, i2);
        fail = false;
        lookup.emplace(std::make_pair(i1, i2));
        fail = true;
    } catch(...) {
        assert(bin == tester);
    }

}
/****/

int main() {
    std::srand(std::time(nullptr));
    binder<int, string> victim;
    binder<int, string> tester;
    std::map<int, string> lookup;

    fail = true;

    for (int i = 0; i < 100000; i++) {
        snapshot(tester, victim);
        //if (victim.size() > 0)
        int choice = std::rand() % 4;
        try {
            //std::cout << victim.size() << std::endl;
            switch (choice) {
            case 0:
                victim.insert_front(i, std::to_string(i));
                break;
            case 1:
                victim.remove();
                break;
            case 2:
                testDel(victim, lookup);
                break;
            case 3:
                testIns(victim, lookup, i, std::to_string(i));
            }
            
        } catch (...) {
            assert(victim == tester);
            //std::cout << "exception caught and handled properly\n";
        }
    }
    std::cout << victim.size() << std::endl;
    auto it2 = victim.cbegin();
    for(int i = 0; i < victim.size(); i++) {
        std::cout << *it2 << std::endl;
        it2++;
    }

    return 0;


}
