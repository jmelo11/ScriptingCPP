#include <iostream>
#include <algorithm>
#include "MemoryManager.h"

int main() {
    const char* msg = "Hello";
    LPSTR mem = MGetTempMemory(6);
    if(mem) {
        std::copy(msg, msg+6, mem);
        std::cout << "Allocated message: " << mem << std::endl;
    } else {
        std::cout << "Allocation failed" << std::endl;
    }
    MFreeAllTempMemory();
    return 0;
}
