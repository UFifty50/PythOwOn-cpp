#ifndef VM_HPP
#define VM_HPP

#include <string>


class VM {
    private:
        Chunk* chunk;

    public:
        VM();    // initVM()
        ~VM();   // freeVM()
};

#endif
