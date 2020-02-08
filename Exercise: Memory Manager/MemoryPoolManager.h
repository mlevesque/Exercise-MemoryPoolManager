//
//  MemoryPoolManager.h
//  Exercise: Memory Manager
//
//  Created by Michael Levesque on 2/3/20.
//  Copyright © 2020 Michael Levesque. All rights reserved.
//

#ifndef MemoryPoolManager_h
#define MemoryPoolManager_h

#include <cstdint>
#include <exception>
#include <algorithm>

/// Exception class for exceptions thrown in the memory manager.
class MemoryPoolException : public std::exception {
    
private:
    template <class T>
    friend class MemoryPoolManager;
    
    // Exception strings
    static const char* invalidSizeMsg;
    static const char* invalidFreedAddressMsg;
    static const char* memoryCorruptionMsg;
    static const char* duplicateFreeMsg;
    
    const char* _msg;
public:
    MemoryPoolException(const char* msg);
    const char* what() const throw();
};


/// Memory Manager for managing blocks of memory for a templated type.
template <class T>
class MemoryPoolManager {
private:
    // Padding type (and set data size) between blocks
    typedef uint16_t padding;
    
    /// Data pattern for padding between blocks. Used for memory corruption checking.
    const static uint16_t paddingSignature = 0xBEEF;
    
    /// Structure for building a linked list of memory pages or blocks
    struct Link {
        Link* next;
    };
    
    const unsigned int _blocksPerPage;
    const unsigned int _blockSize;
    
    /// Linked list of all allocated pages of memory
    Link* _memoryPages;
    
    // Linked list of all available blocks in all pages of memory
    Link* _availableBlocks;
    
    unsigned int _numberOfPages;
    unsigned int _blocksRemaining;
    
    
    /// Allocates a new page of memory, adds it to the page linked list, and sets up all the blocks in the page.
    void allocatePage() {
        unsigned pageAllocationSize = sizeof(Link) + _blockSize * _blocksPerPage;
#ifdef VALIDATIONS_ENABLED
        pageAllocationSize += sizeof(padding) * (_blocksPerPage + 1);
#endif
        // allocate page and add to list
        Link* page = reinterpret_cast<Link*>(malloc(pageAllocationSize));
        page->next = _memoryPages;
        _memoryPages = page;
        
        // setup blocks
        Link* block;
        char* pos = reinterpret_cast<char*>(page) + sizeof(Link); // position pointer past page linked list data
        for (int i = 0; i < _blocksPerPage; ++i) {
#ifdef VALIDATIONS_ENABLED
            // set padding signature
            *reinterpret_cast<padding*>(pos) = paddingSignature;
            pos += sizeof(padding);
#endif
            
            // add block to list
            block = reinterpret_cast<Link*>(pos);
            block->next = _availableBlocks;
            _availableBlocks = block;
            pos += _blockSize;
        }
        
#ifdef VALIDATIONS_ENABLED
        // set padding signature at the end
        *reinterpret_cast<padding*>(pos) = paddingSignature;
#endif
        
        // update values
        ++_numberOfPages;
        _blocksRemaining += _blocksPerPage;
    }
    
#ifdef VALIDATIONS_ENABLED
    /// Checks if given block to be freed is at a valid memory address of where a block should be on any of the
    /// allocated pages. Will thrown an exception if it is not valid.
    /// @param blockToFree The block of memory to validate against.
    void validateBlockLocation(char* blockToFree) {
        Link* page = _memoryPages;
        unsigned fullBlockSize = _blockSize + sizeof(padding);
        std::ptrdiff_t blockDistance;
        char* blockStartPosition;
        bool foundValidLocation = false;
        
        // iterate through each page and check if the given block address is positioned where a block *should* be
        while (!foundValidLocation && page) {
            // the address location of the first block on a page is after the next page link pointer and the first
            // few bytes of data signature padding
            blockStartPosition = reinterpret_cast<char*>(page) + sizeof(Link) + sizeof(padding);
            
            // calculate the difference between the given block location and the location of the first block in the page
            // if the difference is positive or zero, and is divisible by the size of a block plus the size of the
            // padding after it, then the given block pointer is at the correct location
            blockDistance = blockToFree - blockStartPosition;
            foundValidLocation = blockDistance >= 0
                && blockDistance / fullBlockSize < _blocksPerPage
                && blockDistance % fullBlockSize == 0;
            
            // go to next page of memory
            page = page->next;
        }
        
        if (!foundValidLocation) {
            throw MemoryPoolException(MemoryPoolException::invalidFreedAddressMsg);
        }
    }
    
    /// Checks if the padding before and after the given block to be freed still retains the expected data signature.
    /// If it doesn't, then this will throw an exception.
    /// @param blockToFree The block of memory to validate against.
    void validateMemoryCorruption(char* blockToFree) {
        char* pos = blockToFree - sizeof(padding);
        unsigned int fullBlockSize = _blockSize + sizeof(padding);
        if (*reinterpret_cast<padding*>(pos) != paddingSignature
            || *reinterpret_cast<padding*>(pos+fullBlockSize) != paddingSignature) {
            throw MemoryPoolException(MemoryPoolException::memoryCorruptionMsg);
        }
    }
    
    /// Checks if the given block to be freed is already in the linked list of available blocks. If it is, then this
    /// means that the block is already freed and cannot be freed again, so this will throw an exception.
    /// @param blockToFree The block of memory to validate against.
    void validateMultiFree(char* blockToFree) {
        Link* block = reinterpret_cast<Link*>(blockToFree);
        Link* pList = _availableBlocks;
        while (pList && pList != block) {
            pList = pList->next;
        }
        if (pList) {
            throw MemoryPoolException(MemoryPoolException::duplicateFreeMsg);
        }
    }
#endif
    
public:
    /// Constructor.
    /// @param blocksPerPage Number of individual blocks of size T for each allocated page of memory. If this is zero,
    ///     then an exception will be thrown.
    MemoryPoolManager<T>(const unsigned int blocksPerPage)
    : _blocksPerPage(blocksPerPage)
    , _blockSize(std::max(sizeof(T), sizeof(void*))) // block size must be at least big enough to store a pointer
    , _memoryPages(nullptr)
    , _availableBlocks(nullptr)
    , _numberOfPages(0)
    , _blocksRemaining(0) {
        // check for invalid block count
        if (_blocksPerPage == 0) {
            throw MemoryPoolException(MemoryPoolException::invalidSizeMsg);
        }
        
        // allocate initial page
        allocatePage();
    }
    
    /// Destructor
    ~MemoryPoolManager() {
        clearAllMemory();
    }
    
    const unsigned int getBlocksPerPage() {return _blocksPerPage;}
    const unsigned int getNumberOfPages() {return _numberOfPages;}
    const unsigned int getAvailableBlocksRemaining() {return _blocksRemaining;}
    
    
    /// Returns an available block from one of the memory pages. If there are no more available, then a new page will be
    ///  allocated.
    T* allocateBlock() {
        // allocate a new page if no more available blocks
        if (!_availableBlocks) {
            allocatePage();
        }
        
        // pop block
        Link* block = _availableBlocks;
        _availableBlocks = block->next;
        
        // update values
        --_blocksRemaining;
        
        return reinterpret_cast<T*>(block);
    }
    
    
    /// Returns an allocated block back to the memory manager pool. If performValidations param is passed as true, then
    /// validation checks will be performs and can throw exceptions if the given block is invalid or if buffer overflow/
    /// underflow has occurred with the block.
    /// @param block The block to free up.
    void freeBlock(T* block) {
        if (block) {

#ifdef VALIDATIONS_ENABLED
            // perform validation checks on block pointer
            char* blockBytes = reinterpret_cast<char*>(block);
            validateBlockLocation(blockBytes);
            validateMemoryCorruption(blockBytes);
            validateMultiFree(blockBytes);
#endif
            
            // push block back to list
            Link* blockLink = reinterpret_cast<Link*>(block);
            blockLink->next = _availableBlocks;
            _availableBlocks = blockLink;
            
            // update values
            ++_blocksRemaining;
        }
    }
    
    /// Deallocates all memory page allocations. Any allocated blocks from this memory manage will be invalid.
    void clearAllMemory() {
        Link* pList = _memoryPages;
        Link* pageToDealloc;
        while (pList) {
            pageToDealloc = pList;
            pList = pList->next;
            free(pageToDealloc);
        }
        _memoryPages = _availableBlocks = nullptr;
        _numberOfPages = _blocksRemaining = 0;
    }
};

#endif /* MemoryPoolManager_h */
