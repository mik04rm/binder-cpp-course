#ifndef troll
#define troll
int counter = 0;
#include <memory>
#include <stdexcept>
#include <cstdlib>

class Random {
private:
    bool shouldThrow = false;

public:
    int value;
    Random(int i) : shouldThrow(false), value(i) {
        counter++;
        if (shouldThrow || counter % 4 == 3)
            throw std::invalid_argument("xd");
    }
    Random(Random const& other) :shouldThrow(false), value(other.value){
        counter++;
        if (shouldThrow || counter % 4 == 3)
            throw std::invalid_argument("xd");
    }
    Random(Random&& other) :shouldThrow(false), value(std::move(other.value)) {
        counter++;
        if (shouldThrow || counter % 4 == 3)
            throw std::invalid_argument("xd");
    }

    void setFlag(bool f) {
        shouldThrow = f;
    }
    Random(int i, bool a) : shouldThrow(a), value(i) { }

    bool operator== (const Random& r1) const { return this->value == r1.value; }
    bool operator!= (const Random& r1) const { return this->value != r1.value; }
    bool operator>= (const Random& r1) const { return this->value >= r1.value; }
    bool operator<= (const Random& r1) const { return this->value <= r1.value; }
    bool operator> (const Random& r1) const { return this->value > r1.value; }
    bool operator< (const Random& r1) const { return this->value < r1.value; }


};

#endif