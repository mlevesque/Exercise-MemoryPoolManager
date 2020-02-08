//
//  MemoryPoolManager.cpp
//  Exercise: Memory Manager
//
//  Created by Michael Levesque on 2/3/20.
//  Copyright © 2020 Michael Levesque. All rights reserved.
//

#include "MemoryPoolManager.h"

/// Constructor for Memory Manager Exception.
/// @param msg Exception message.
MemoryPoolException::MemoryPoolException(const char* msg)
: _msg(msg) {}

/// Returns message for exception.
const char* MemoryPoolException::what() const throw() {
    return _msg;
}

const char* MemoryPoolException::invalidSizeMsg = "Invalid block size for Fixed Size Memory Manager.";
const char* MemoryPoolException::invalidFreedAddressMsg = "Invalid address location for the freed block.";
const char* MemoryPoolException::memoryCorruptionMsg = "Memory corruption has been detected.";
const char* MemoryPoolException::duplicateFreeMsg = "Memory Block has already been freed.";
