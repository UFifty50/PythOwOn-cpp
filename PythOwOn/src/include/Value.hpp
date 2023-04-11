#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>

#include "Common.hpp"


typedef double Value;
//typedef std::variant<int, double, std::string> Value;
typedef std::vector<Value> ValueArray;

void printValue(Value value);

template <typename T>
class Stack : private std::vector<T> {
public:

    inline Value peek(int distance) { return this->at(this->size() - 1 - distance); }
    
    inline Value pop() {
        Value value = peek(0);
        this->pop_back();
        return value;
    }
    
    inline void push(T value) { this->emplace_back(value); }

    std::vector<T>::iterator begin() { return this->std::vector<T>::begin(); }
    std::vector<T>::iterator end() { return this->std::vector<T>::end(); }
};

#endif // !VALUE_HPP
