//
//  profiling.cpp
//  Exercise: Memory Manager
//
//  Created by Michael Levesque on 2/7/20.
//  Copyright © 2020 Michael Levesque. All rights reserved.
//

#include "profiling.h"
#include "MemoryPoolManager.h"
#include <cstdlib>
#include <chrono>
#include <vector>
#include <iostream>

template <class T>
void performMalloc(const unsigned numberOfAllocations) {
    T** blocks = reinterpret_cast<T**>(malloc(sizeof(T*) * numberOfAllocations));
    for (int i = 0; i < numberOfAllocations; ++i) {
        blocks[i] = reinterpret_cast<T*>(malloc(sizeof(T)));
    }
    for (int i = 0; i < numberOfAllocations; ++i) {
        free(blocks[i]);
    }
    free(blocks);
}

template <class T>
void performMemoryManagerAllocations(const unsigned numberOfAllocations, const unsigned blocksPerPage) {
    T** blocks = reinterpret_cast<T**>(malloc(sizeof(T*) * numberOfAllocations));
    MemoryPoolManager<T> manager(blocksPerPage);
    for (int i = 0; i < numberOfAllocations; ++i) {
        blocks[i] = manager.allocateBlock();
    }
    for (int i = 0; i < numberOfAllocations; ++i) {
        manager.freeBlock(blocks[i]);
    }
    free(blocks);
}

template <class T>
void profileMemoryManagerAllocations(const unsigned numberOfAllocations, const std::vector<unsigned> blocksPerPage) {
    std::cout << ">>> Profiling with " << numberOfAllocations << " allocations <<<" << std::endl;
    std::cout << "Malloc: ";
    auto start = std::chrono::system_clock::now();
    performMalloc<T>(numberOfAllocations);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << diff.count() << " s" << std::endl;
    
    for (auto i = blocksPerPage.begin(); i != blocksPerPage.end(); ++i) {
        std::cout << "Memory Manager with " << *i << " blocks per page: ";
        start = std::chrono::system_clock::now();
        performMemoryManagerAllocations<T>(numberOfAllocations, *i);
        end = std::chrono::system_clock::now();
        diff = end - start;
        std::cout << diff.count() << " s" << std::endl;
    }
}

void profileMemoryManger() {
    std::vector<unsigned> blocksPerPage{10, 100, 1000};
    profileMemoryManagerAllocations<int>(1000, blocksPerPage);
    blocksPerPage.clear();
    std::cout << std::endl;
    
    blocksPerPage = {10, 100, 1000, 10000};
    profileMemoryManagerAllocations<int>(10000, blocksPerPage);
    blocksPerPage.clear();
    std::cout << std::endl;
    
    blocksPerPage = {10, 100, 1000, 10000};
    profileMemoryManagerAllocations<int>(100000, blocksPerPage);
    blocksPerPage.clear();
    std::cout << std::endl;
}
