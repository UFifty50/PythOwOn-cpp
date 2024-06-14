#ifndef STACK_HPP
#define STACK_HPP

#include <vector>


template <typename T>
class Stack : std::vector<T> {
public:
    [[nodiscard]] T peek(uint32_t distance) const {
        return this->at(this->size() - 1 - distance);
    }

    T pop() {
        T value = peek(0);
        this->pop_back();
        return value;
    }

    void push(T value) { this->emplace_back(value); }
    void reset() { this->clear(); }

    using std::vector<T>::operator[];

    using std::vector<T>::size;
    using std::vector<T>::empty;
    using std::vector<T>::begin;
    using std::vector<T>::end;
};

#endif
