//
//  main.cpp
//  Exercise: Memory Manager
//
//  Created by Michael Levesque on 2/3/20.
//  Copyright © 2020 Michael Levesque. All rights reserved.
//

#include <iostream>
#include "MemoryPoolManager.h"
#include "test_cases.h"
#include "profiling.h"

int main(int argc, const char * argv[]) {
    std::cout << "Performing test cases for Memory Manager..." << std::endl << std::endl;
    testMemoryManager();
    std::cout << std::endl;
    profileMemoryManger();
    return 0;
}
