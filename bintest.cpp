#include "binderT.h"
#include "binder.h"
#include <cassert>
#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <ctime>


using std::invalid_argument;
using std::pair;
using std::make_pair;
using std::string;
using cxx::binder;

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

int main() {
    std::srand(std::time(nullptr));

    binder<Random, string> b1;
    b1.insert_front(Random(3, false), "first");
    //b1.insert_front(Random(4, false), "second");
    binder<Random, string> b2;
    //for (int i =0; i < 100; i++)
        b2.copyFrom(b1);
    

    try {
        assert(b1 == b2);
        
    } catch (...) {
        assert(b1 == b2);

    }

    return 0;


}