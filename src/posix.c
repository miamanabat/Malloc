/* posix.c: POSIX API Implementation */

#include "malloc/counters.h"
#include "malloc/freelist.h"

#include <assert.h>
#include <string.h>

/**
 * Allocate specified amount memory.
 * @param   size    Amount of bytes to allocate.
 * @return  Pointer to the requested amount of memory.
 **/
void *malloc(size_t size) {
    // Initialize counters
    init_counters();

    // Handle empty size
    if (!size) {
        return NULL;
    }

    // TODO: Search free list for any available block with matching size
    
    Block * block = free_list_search(size); 

    // Could not find free block or allocate a block, so just return NULL
    if (!block) {
        block = block_allocate(size);  
    } else {
        block = block_split(block, size);
        block = block_detach(block); 
    }

    if (!block) {
        return NULL;
    }


    // Check if allocated block makes sense
    assert(block->capacity >= block->size); // this is when it aborts
    assert(block->size     == size); // this is when it aborts
    assert(block->next     == block);
    assert(block->prev     == block);

    // Update counters
    Counters[MALLOCS]++;
    Counters[REQUESTED] += size;

    // Return data address associated with block
    return block->data;
}

/**
 * Release previously allocated memory.
 * @param   ptr     Pointer to previously allocated memory.
 **/
void free(void *ptr) {
    if (!ptr) {
        return;
    }

    // Update counters
    Counters[FREES]++;

    // TODO: Try to release block, otherwise insert it into the free list
    Block * block = BLOCK_FROM_POINTER(ptr);
    if (!block_release(block)) {
       free_list_insert(block); 
    } 
}

/**
 * Allocate memory with specified number of elements and with each element set
 * to 0.
 * @param   nmemb   Number of elements.
 * @param   size    Size of each element.
 * @return  Pointer to requested amount of memory.
 **/
void *calloc(size_t nmemb, size_t size) {
    // TODO: Implement calloc

    // TODO: Search free list for any available block with matching size
    // Only need the data from malloc to ensure memset does not override other important block info
    void * block = malloc(nmemb*size); 
    if (!block) return NULL;

    memset(block, 0, nmemb*size);

    // Update counters
    Counters[CALLOCS]++;

    // Return data address associated with block
    return block;
}

/**
 * Reallocate memory with specified size.
 * @param   ptr     Pointer to previously allocated memory.
 * @param   size    Amount of bytes to allocate.
 * @return  Pointer to requested amount of memory.
 **/
void *realloc(void *ptr, size_t size) {
    // TODO: Implement realloc
   
    // Update counters
    Counters[REALLOCS]++; 
    if (!ptr) return malloc(size);

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    Block *old_block = BLOCK_FROM_POINTER(ptr);

    // If old size is larger than new, no new memory needed to be allocated
    if (old_block->capacity >= size) {
        old_block->size = size;
        return old_block->data;
    } else {
        Block * block = BLOCK_FROM_POINTER(malloc(size));
 
        char * prev = old_block->data;
        for (char *curr = block->data; curr < block->data + size; curr++) {
            *curr = *prev;
            prev += 1;
        }
            free(ptr);
    
        // Return data address associated with block
        return block->data;
    }

    return NULL;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
