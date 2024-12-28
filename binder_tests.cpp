#include <iostream>
#include <stdexcept>
#include <cassert>
#include "binder.h"

void test_default_constructor() {
    cxx::binder<int, std::string> b;
    assert(b.size() == 0);
    std::cout << "test_default_constructor passed.\n";
}

void test_insert_front() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");
    assert(b.size() == 1);
    assert(b.read(1) == "Note1");

    // Test duplicate key insertion
    try {
        b.insert_front(1, "Note2");
        assert(false && "Expected exception not thrown.");
    } catch (const std::invalid_argument&) {
        std::cout << "Duplicate key insertion correctly throws exception.\n";
    }

    std::cout << "test_insert_front passed.\n";
}

void test_insert_after() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");
    b.insert_after(1, 2, "Note2");
    assert(b.size() == 2);
    assert(b.read(2) == "Note2");

    // Test invalid prev_k
    try {
        b.insert_after(3, 4, "Note4");
        assert(false && "Expected exception not thrown.");
    } catch (const std::invalid_argument&) {
        std::cout << "Invalid prev_k correctly throws exception.\n";
    }

    std::cout << "test_insert_after passed.\n";
}

void test_remove_front() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");
    b.remove();
    assert(b.size() == 0);

    // Test removal from empty binder
    try {
        b.remove();
        assert(false && "Expected exception not thrown.");
    } catch (const std::invalid_argument&) {
        std::cout << "Removing from empty binder correctly throws exception.\n";
    }

    std::cout << "test_remove_front passed.\n";
}

void test_remove_key() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");
    b.insert_front(2, "Note2");
    b.remove(2);
    assert(b.size() == 1);

    // Test removing a non-existing key
    try {
        b.remove(3);
        assert(false && "Expected exception not thrown.");
    } catch (const std::invalid_argument&) {
        std::cout << "Removing non-existent key correctly throws exception.\n";
    }

    std::cout << "test_remove_key passed.\n";
}

void test_read() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");

    // Read and modify
    b.read(1) = "ModifiedNote1";
    assert(b.read(1) == "ModifiedNote1");

    // Test const version
    const cxx::binder<int, std::string>& cb = b;
    assert(cb.read(1) == "ModifiedNote1");

    // Test reading a non-existent key
    try {
        cb.read(2);
        assert(false && "Expected exception not thrown.");
    } catch (const std::invalid_argument&) {
        std::cout << "Reading non-existent key correctly throws exception.\n";
    }

    std::cout << "test_read passed.\n";
}

void test_copy_on_write() {
    cxx::binder<int, std::string> b1;
    b1.insert_front(1, "Note1");

    cxx::binder<int, std::string> b2 = b1; // Shared state
    b2.insert_front(2, "Note2");
    b2.read(1) = "ModifiedNote1";          

    // Verify copy-on-write
    assert(b1.read(1) == "Note1");
    assert(b2.read(1) == "ModifiedNote1");

    std::cout << "test_copy_on_write passed.\n";
}


void test_copy_on_write_2() {
    cxx::binder<int, std::string> b1;
    b1.insert_front(1, "Note1");

    cxx::binder<int, std::string> b2 = b1; // Shared state
    b2.read(1) = "ModifiedNote1";      

    // Verify copy-on-write
    assert(b1.read(1) == "Note1");
    assert(b2.read(1) == "ModifiedNote1");

    std::cout << "test_copy_on_write_2 passed.\n";
}

void test_copy_on_write_3() {
    cxx::binder<int, std::string> b1;
    b1.insert_front(1, "Note1");
    b1.read(1) = "ModifiedNote1";

    cxx::binder<int, std::string> b2 = b1; // Shared state
    assert(&b1.read(1) != &b2.read(1));
    assert(b1.read(1) == b2.read(1));

    std::cout << "test_copy_on_write_3 passed.\n";
}

void test_clear() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");
    b.insert_front(2, "Note2");

    b.clear();
    assert(b.size() == 0);

    // Test read after clear
    try {
        b.read(1);
        assert(false && "Expected exception not thrown.");
    } catch (const std::invalid_argument&) {
        std::cout << "Reading after clear correctly throws exception.\n";
    }

    std::cout << "test_clear passed.\n";
}

void test_iterator() {
    cxx::binder<int, std::string> b;
    b.insert_front(1, "Note1");
    b.insert_front(2, "Note2");
    b.insert_front(3, "Note3");

    // Test forward iteration
    auto it = b.cbegin();
    assert(*it == "Note3");
    ++it;
    assert(*it  == "Note2");
    ++it;
    assert(*it  == "Note1");

    // Test end condition
    ++it;
    assert(it == b.cend());

    std::cout << "test_iterator passed.\n";
}

int main() {
    test_default_constructor();
    test_insert_front();
    test_insert_after();
    test_remove_front();
    test_remove_key();
    test_read();
    test_copy_on_write();
    test_copy_on_write_2();
    test_copy_on_write_3();
    test_clear();
    test_iterator();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
