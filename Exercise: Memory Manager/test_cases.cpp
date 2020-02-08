//
//  test_cases.cpp
//  Exercise: Memory Manager
//
//  Created by Michael Levesque on 2/5/20.
//  Copyright © 2020 Michael Levesque. All rights reserved.
//

#include "test_cases.h"
#include "MemoryPoolManager.h"
#include <string>
#include <iostream>
#include <list>

/// Conditions for if results should be outputted
enum RecordResultsCondition {
    NoOutput = 0,
    PassOnly = 1,
    FailOnly = 2,
    AnyResult = 3
};

/// What type of exception may or may not have been caught when performing an action with the Memory Manager.
enum ManagerExceptionType {
    NoException,
    KnownException,
    UnknownException
};

/// Just a test dummy object
struct DummyObject {
    int value;
    char myString[10];
    bool boolVal;
};

struct TestResult {
    TestResult(std::string title)
    : title(title)
    , description("")
    , isPassed(false)
    , resultFound(false) {}
    
    void setResult(const bool pass, const std::string desc = "") {
        isPassed = pass;
        description = desc;
        resultFound = true;
    }
    
    std::string title;
    std::string description;
    bool isPassed;
    bool resultFound;
};

void outputTestResult(TestResult& results) {
    std::string result = results.resultFound ? (results.isPassed ? "[PASS] " : "[FAIL] ") : "[NO RESULT] ";
    std::cout << result << results.title;
    if (results.description != "") {
        std::cout << " - " << results.description;
    }
    std::cout << std::endl;
}

bool shouldRecordResults(bool isPassed, RecordResultsCondition outputFlag) {
    RecordResultsCondition actual = isPassed ? PassOnly : FailOnly;
    return (actual & outputFlag) == actual;
}

template <class T>
MemoryPoolManager<T>* createManager(  const unsigned blocksPerPage,
                                      bool expectManagerException,
                                      RecordResultsCondition resultsCondition,
                                      TestResult& result  ) {
    MemoryPoolManager<T>* manager = nullptr;
    ManagerExceptionType exceptionType = NoException;
    try {
        manager = new MemoryPoolManager<T>(blocksPerPage);
    }
    catch (const MemoryPoolException& e) {
        exceptionType = KnownException;
    }
    catch (...) {
        exceptionType = UnknownException;
    }
    if (shouldRecordResults(true, resultsCondition)
        && ((expectManagerException && exceptionType == KnownException)
            || (!expectManagerException && exceptionType == NoException))) {
        result.setResult(true);
    }
    else if (shouldRecordResults(false, resultsCondition)) {
        if (expectManagerException) {
            if (exceptionType == NoException) {
                result.setResult(false, "Constructor did not throw an expected exception.");
            }
            else if (exceptionType == UnknownException) {
                result.setResult(false, "Constructor threw an unexpected exception.");
            }
        }
        else if (exceptionType != NoException) {
            result.setResult(false, "Constructor threw an unexpected exception.");
        }
    }
    return manager;
}

template <class T>
T* allocateBlock(   MemoryPoolManager<T>* manager,
                    bool expectManagerException,
                    RecordResultsCondition resultsCondition,
                    TestResult& result ) {
    T* block = nullptr;
    ManagerExceptionType exceptionType = NoException;
    try {
        block = manager->allocateBlock();
    }
    catch (const MemoryPoolException& e) {
        exceptionType = KnownException;
    }
    catch (...) {
        exceptionType = UnknownException;
    }
    if (shouldRecordResults(true, resultsCondition)
        && ((expectManagerException && exceptionType == KnownException)
            || (!expectManagerException && exceptionType == NoException))) {
        result.setResult(true);
    }
    else if (shouldRecordResults(false, resultsCondition)) {
        if (expectManagerException) {
            if (exceptionType == NoException) {
                result.setResult(false, "Allocation did not throw an expected exception.");
            }
            else if (exceptionType == UnknownException) {
                result.setResult(false, "Allocation threw an unexpected exception.");
            }
        }
        else if (exceptionType != NoException) {
            result.setResult(false, "Allocation threw an unexpected exception.");
        }
    }
    return block;
}

template <class T>
void freeBlock( MemoryPoolManager<T>* manager,
                T* block,
                bool expectManagerException,
                RecordResultsCondition printOutput,
                TestResult& result ) {
    ManagerExceptionType exceptionType = NoException;
    try {
        manager->freeBlock(block);
    }
    catch (const MemoryPoolException& e) {
        exceptionType = KnownException;
    }
    catch (...) {
        exceptionType = UnknownException;
    }
    if (shouldRecordResults(true, printOutput)
        && ((expectManagerException && exceptionType == KnownException)
            || (!expectManagerException && exceptionType == NoException))) {
        result.setResult(true);
    }
    else if (shouldRecordResults(false, printOutput)) {
        if (expectManagerException) {
            if (exceptionType == NoException) {
                result.setResult(false, "Deallocation did not throw an expected exception.");
            }
            else if (exceptionType == UnknownException) {
                result.setResult(false, "Deallocation threw an unexpected exception.");
            }
        }
        else if (exceptionType != NoException) {
            result.setResult(false, "Deallocation threw an unexpected exception.");
        }
    }
}

void writeIntToBlock( int* block, int value, RecordResultsCondition resultsCondition, TestResult& result) {
    bool pass = true;
    try {
        *block = value;
    }
    catch (...) {
        pass = false;
    }
    if (!pass && shouldRecordResults(false, resultsCondition)) {
        result.setResult(false, "Writing caused unexpected exception.");
    }
    if (pass && shouldRecordResults(true, resultsCondition)) {
        result.setResult(true);
    }
}

template <class T>
void writeCorruption(bool useUnderflow, int offset, TestResult& result) {
    auto manager = createManager<T>(10, false, FailOnly, result);
    T* block = nullptr;
    if (!result.resultFound) block = allocateBlock<T>(manager, false, FailOnly, result);
    char* offsetBlock = reinterpret_cast<char*>(block);
    if (useUnderflow) {
        offsetBlock -= offset;
    }
    else {
        offsetBlock += std::max(sizeof(T), sizeof(void*)) + offset;
    }
    *reinterpret_cast<int*>(offsetBlock) = 0x1234;
    if (!result.resultFound) freeBlock(manager, block, true, AnyResult, result);
    delete manager;
}

template <class T>
void testConstruction() {
    TestResult result("Successful Construction");
    auto manager = createManager<T>(10, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Unsuccessful Construction");
    manager = createManager<T>(0, true, AnyResult, result);
    delete manager;
    outputTestResult(result);
}

template <class T>
void testAllocation() {
    TestResult result("Single Block Allocation");
    auto manager = createManager<T>(5, false, FailOnly, result);
    if (!result.resultFound) allocateBlock<T>(manager, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Multiple Block Allocation");
    manager = createManager<T>(5, false, FailOnly, result);
    for (int i = 0; i < 4; ++i) {
        if (!result.resultFound) allocateBlock<T>(manager, false, FailOnly, result);
    }
    if (!result.resultFound) allocateBlock<T>(manager, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Multiple Page Allocation");
    manager = createManager<T>(5, false, FailOnly, result);
    for (int i = 0; i < 9; ++i) {
        if (!result.resultFound) allocateBlock<T>(manager, false, FailOnly, result);
    }
    if (!result.resultFound) allocateBlock<T>(manager, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
}

void testWritingIntToBlock() {
    TestResult result("Single Block Write");
    auto manager = createManager<int>(5, false, FailOnly, result);
    int* block = nullptr;
    if (!result.resultFound) block = allocateBlock<int>(manager, false, FailOnly, result);
    if (!result.resultFound) writeIntToBlock(block, 42, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Multiple Block Write");
    manager = createManager<int>(5, false, FailOnly, result);
    for (int i = 0; i < 4; ++i) {
        if (!result.resultFound) block = allocateBlock<int>(manager, false, FailOnly, result);
        if (!result.resultFound) writeIntToBlock(block, i, FailOnly, result);
    }
    if (!result.resultFound) freeBlock(manager, block, false, FailOnly, result);
    if (!result.resultFound) block = allocateBlock<int>(manager, false, FailOnly, result);
    if (!result.resultFound) writeIntToBlock(block, 42, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Multiple Page Write");
    manager = createManager<int>(5, false, FailOnly, result);
    for (int i = 0; i < 9; ++i) {
        if (!result.resultFound) block = allocateBlock<int>(manager, false, FailOnly, result);
        if (!result.resultFound) writeIntToBlock(block, i, FailOnly, result);
    }
    if (!result.resultFound) block = allocateBlock<int>(manager, false, FailOnly, result);
    if (!result.resultFound) writeIntToBlock(block, 42, AnyResult, result);
    delete manager;
    outputTestResult(result);
}

template <class T>
void testDeallocation() {
    TestResult result("Single Block Deallocation");
    auto manager = createManager<T>(5, false, FailOnly, result);
    T* block = nullptr;
    if (!result.resultFound) block = allocateBlock<T>(manager, false, FailOnly, result);
    if (!result.resultFound) freeBlock<T>(manager, block, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Multiple Block Deallocation");
    manager = createManager<T>(5, false, FailOnly, result);
    std::list<T*> blockList;
    for (int i = 0; i < 5; ++i) {
        if (!result.resultFound) blockList.push_back(allocateBlock<T>(manager, false, FailOnly, result));
    }
    for (int i = 0; i < 4; ++i) {
        block = blockList.back();
        blockList.pop_back();
        if (!result.resultFound) freeBlock<T>(manager, block, false, FailOnly, result);
    }
    block = blockList.back();
    if (!result.resultFound) freeBlock<T>(manager, block, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("NULL Block Deallocation");
    manager = createManager<T>(5, false, FailOnly, result);
    if (!result.resultFound) freeBlock<T>(manager, nullptr, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
}

template <class T>
void testFreeBlockAddressLocation() {
    TestResult result("Valid Block Address");
    auto manager = createManager<T>(10, false, FailOnly, result);
    T* block = nullptr;
    if (!result.resultFound) block = allocateBlock<T>(manager, false, FailOnly, result);
    if (!result.resultFound) freeBlock<T>(manager, block, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Invalid Block Address Offset");
    manager = createManager<T>(10, false, FailOnly, result);
    if (!result.resultFound) block = allocateBlock<T>(manager, false, FailOnly, result);
    char* offsetBlock = reinterpret_cast<char*>(block) + sizeof(T) / 2;
    if (!result.resultFound) freeBlock(manager, reinterpret_cast<T*>(offsetBlock), true, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Invalid Block Address Random");
    manager = createManager<T>(10, false, FailOnly, result);
    offsetBlock = (char*)0x12345678;
    if (!result.resultFound) freeBlock(manager, reinterpret_cast<T*>(offsetBlock), true, AnyResult, result);
    delete manager;
    outputTestResult(result);
}

template <class T>
void testFreeBlockDuplicateFree() {
    TestResult result("Valid Allocation and Dellocation of Same Block");
    auto manager = createManager<T>(1, false, FailOnly, result);
    T* block = nullptr;
    if (!result.resultFound) block = allocateBlock<T>(manager, false, FailOnly, result);
    if (!result.resultFound) freeBlock(manager, block, false, FailOnly, result);
    if (!result.resultFound) block = allocateBlock<T>(manager, false, FailOnly, result);
    if (!result.resultFound) freeBlock(manager, block, false, AnyResult, result);
    delete manager;
    outputTestResult(result);
    
    result = TestResult("Duplicate Free");
    manager = createManager<T>(10, false, FailOnly, result);
    if (!result.resultFound) block = allocateBlock(manager, false, FailOnly, result);
    if (!result.resultFound) freeBlock(manager, block, false, FailOnly, result);
    if (!result.resultFound) freeBlock(manager, block, true, AnyResult, result);
    delete manager;
    outputTestResult(result);
}

template <class T>
void testFreeBlockMemoryCorruption() {
    TestResult result("Buffer Underflow 1");
    writeCorruption<T>(true, 1, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Underflow 2");
    writeCorruption<T>(true, 2, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Underflow 3");
    writeCorruption<T>(true, 3, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Overflow 1");
    writeCorruption<T>(false, -3, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Overflow 2");
    writeCorruption<T>(false, -2, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Overflow 3");
    writeCorruption<T>(false, -1, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Overflow 4");
    writeCorruption<T>(false, 0, result);
    outputTestResult(result);
    
    result = TestResult("Buffer Overflow 5");
    writeCorruption<T>(false, 1, result);
    outputTestResult(result);
}

void testMemoryManager() {
    std::cout << ">>> Int Memory Manager Tests <<<" << std::endl;
    testConstruction<int>();
    testAllocation<int>();
    testDeallocation<int>();
    testWritingIntToBlock();
#ifdef VALIDATIONS_ENABLED
    testFreeBlockAddressLocation<int>();
    testFreeBlockMemoryCorruption<int>();
    testFreeBlockDuplicateFree<int>();
#endif
    
    std::cout << std::endl << ">>> Dummy Object Memory Manager Tests <<<" << std::endl;
    testConstruction<DummyObject>();
    testAllocation<DummyObject>();
    testDeallocation<DummyObject>();
#ifdef VALIDATIONS_ENABLED
    testFreeBlockAddressLocation<DummyObject>();
    testFreeBlockMemoryCorruption<DummyObject>();
    testFreeBlockDuplicateFree<DummyObject>();
#endif
}
