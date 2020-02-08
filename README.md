# Exercise: Memory Pool Manager

This is a simple C++ implementation of a templated Memory Manager designed for rapid allocation and deallocation of fixed-sized blocks that would be more performant than multiple individual allocations via `new` or `malloc`.

## How It Works

The memory manager will allocate large pages of memory from the system, configured by how many blocks should be contained within one single page of memory. It internally maintains a simple linked list for pages and all blocks contained in those pages.

The manager is templatized, so the size of blocks is determined by the type specified for the memory manager. However, due to the way blocks are maintained, the minimum size of a given block will be the size of a pointer (4 bytes on 32-bit systems and 8 bytes on 64-bit systems).

Once created, you call `allocateBlock` to get a pointer to a block to use. When you want to free up the block, call `freeBlock` and the block will be added back to the internal linked list to be reused later. If no more blocks can be given out when `allocateBlock` is called, then a new page is allocated from the system.

![](https://raw.githubusercontent.com/mlevesque/Exercise-MemoryPoolManager/master/figure1.gif "Figure 1")
*Figure 1: Visual reprsentation of the manager's memory layout.*

## Validation Checking

This memory manager has some limited validation checks when a block is freed, such as making sure the given block pointer is pointing to a valid memory address, checking for some buffer underflow/overflow, and if the block is already supposed to be freed.

To validate that the block is a valid block, the manager checks if the given block pointer is pointing to a memory location within one of the allocated memory pages and if that location is aligned to where one of the blocks should be within that page.

I've added two bytes of padding between blocks with a data signature. When a given block is being freed up (and validation checks happen), the manager checks the signatures before and after the block to make sure that data hasn't been written over on them.

In order to check if the block is already been freed up, the manager searches the block linked list to see if it already exists in it.

These validation checks obviously can't cover all potential issues that can occur. I list a number of things that can go wrong [here](#cons).

Validation checks significantly hinder performance, so I decided to wrap the validation code with a preprocessor and set up a separate build target with that preprocessor.

## Profiling

I included some code to profile the performance of the memory manager and compared it with the same number of allocations through `malloc`. The results show that the memory manager is more performant with allocating and deallocating large number of objects. These tests exclude validation checks from the memory manager, as those significantly hinder performance.

```
>>> Profiling with 1000 allocations <<<
Malloc: 0.000174 s
Memory Manager with 10 blocks per page: 3.6e-05 s
Memory Manager with 100 blocks per page: 1.5e-05 s
Memory Manager with 1000 blocks per page: 9e-06 s

>>> Profiling with 10000 allocations <<<
Malloc: 0.00128 s
Memory Manager with 10 blocks per page: 0.000166 s
Memory Manager with 100 blocks per page: 0.00013 s
Memory Manager with 1000 blocks per page: 5.8e-05 s
Memory Manager with 10000 blocks per page: 0.000114 s

>>> Profiling with 100000 allocations <<<
Malloc: 0.014324 s
Memory Manager with 10 blocks per page: 0.00226 s
Memory Manager with 100 blocks per page: 0.001042 s
Memory Manager with 1000 blocks per page: 0.000544 s
Memory Manager with 10000 blocks per page: 0.001024 s
```

## Pros

- **Better performance for large and rapid object allocation.**
    - Perfect for things like game objects and particles in a particle system.
- **Blocks are contiguous in memory.**
    - We can get performance gains from memory caching.
- **Templatized, so the manager will cast the block pointers for you.**
    - No need for the client to cast the pointer they get when calling `allocateBlock`.

## Cons

- **Any code changes to the memory manager means more of the application's code would need to be recompiled.** 
    - Because it is templatized, if any code changes are needed here, every file referencing the memory manager will need to be re-compiled. Implementation for a template class needs to be in the header file (or the header file must include the implementation file) in order for the implementation of any specific type can be created.
    - Implementation can be moved to a cpp file if I can go ahead and explicitly instantiate all the template instances needed, but I would prefer it to be open to any and all types.
- **When freeing a block, the client can still attempt to use it without anything to stop them.**
    - If the client writes to the block after freeing it, it will break the linked list keeping track of all available blocks since the block itself contains the next pointer for the next block in the linked list.
- **Buffer overflow and underflow can still happen.**
    - With validation turned on, it will only check the 2 bytes of padding before and after a block, and only when that block is being freed. If data is written to only bytes beyond that in either direction, the validation won't detect it and may cause unexpected and hard to debug issues where other blocks will become corrupted.
- **Validations are very slow.**
    - Through my profiling, I observed a significant hit to performance with large numbers of allocations and deallocations with validations turned on compared to using `malloc`, by a factor of 100 to 200.
    - Validation code is wrapped with a preprocessor check, so only a build with that preprocessor will perform them. This of course means that without the preprocessor, no validations will be done and if memory corruption occures or bad pointers are given to the memory manager, things will break and it may be hard to debug the cause.
- **Will not invoke constructors and destructors.**
    - Client code would need to make separate methods for proper object construction and destruction and be responsible for making sure those are called correctly.

## Ways to Potentially Improve It

- **Use handles instead of pointers.**
    - Helps keep the actual pointer to a block intact.
    - Handle can be invalidated when it is freed to prevent the client from accessing it after the fact.
    - However, this will add an extra level of indirection when accessing an object allocated from the memory manager.
- **Explicitly call constructors and destructors.**
    - When a block is allocated and deallocated, we can try explicitly calling the constructor and destructor, respectively.
- **Use separate dedicated containers for blocks and pages instead of using the blocks and pages themselves.**
    - Instead of using the blocks to store points to the next blocks, make a seaparate container and store pointers to these blocks.
    - This would eliminate the minimal size of blocks since they don't need to be big enough to contain pointers to other blocks.
    - Separate containers can mean blocks and pages can be stored in different ways. Blocks could then be stored as a set for quicker duplicate freeing validation checking.
    - Would introduce more memory overhead.
