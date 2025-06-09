#include <iostream>
#include <cstring>
#include "MemoryManager.h"

int main() {
    // Allocate a small buffer using the custom memory manager
    LPSTR buffer = MGetTempMemory(16);
    if (!buffer) {
        std::cerr << "Allocation failed" << std::endl;
        return 1;
    }

    const char* msg = "Hello";
    std::memcpy(buffer, msg, std::strlen(msg) + 1);

    std::cout << "Allocated message: " << buffer << std::endl;

    // Release the temporary memory
    MFreeAllTempMemory();
    return 0;
}
