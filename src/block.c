/* block.c: Block Structure */

#include "malloc/block.h"
#include "malloc/counters.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Allocate a new block on the heap using sbrk:
 *
 *  1. Determined aligned amount of memory to allocate.
 *  2. Allocate memory on the heap.
 *  3. Set allocage block properties.
 *
 * @param   size    Number of bytes to allocate.
 * @return  Pointer to data portion of newly allocate block.
 **/
Block *	block_allocate(size_t size) {
    // Allocate block
    intptr_t allocated = sizeof(Block) + ALIGN(size);
    Block *  block     = sbrk(allocated);
    if (block == SBRK_FAILURE) {
    	return NULL;
    }

    // Record block information
    block->capacity = ALIGN(size);
    block->size     = size;
    block->prev     = block;
    block->next     = block;

    // Update counters
    Counters[HEAP_SIZE] += allocated;
    Counters[BLOCKS]++;
    Counters[GROWS]++;
    return block;
}

/**
 * Attempt to release memory used by block to heap:
 *
 *  1. If the block is at the end of the heap.
 *  2. The block capacity meets the trim threshold.
 *
 * @param   block   Pointer to block to release.
 * @return  Whether or not the release completed successfully.
 **/
bool	block_release(Block *block) {
    //TODO: Implement block release

    if (!block) return false;
    
    // Check that the block capacity meets the trim threshold
    if (block->capacity < TRIM_THRESHOLD) return false;

    // Check that the block is at the end of the heap
    void* top_heap = sbrk(0);
    if (top_heap != block->data + block->capacity) return false; 
    
    size_t allocated = sizeof(Block) + block->capacity;

    Block * block_released = sbrk(-allocated);
    if (block_released == SBRK_FAILURE) return false;
    
    Counters[BLOCKS]--;
    Counters[SHRINKS]++;
    Counters[HEAP_SIZE] -= allocated;

    return true;
}

/**
 * Detach specified block from its neighbors.
 *
 * @param   block   Pointer to block to detach.
 * @return  Pointer to detached block.
 **/
Block * block_detach(Block *block) {
    // TODO: Detach block from neighbors by updating previous and next block
    if (block) {
        Block *before = block->prev;
        Block *after = block->next;

        before->next = block->next;
        after->prev = block->prev;

        block->prev = block;
        block->next = block;
    }

    return block;
}

/**
 * Attempt to merge source block into destination.
 *
 *  1. Compute end of destination and start of source.
 *
 *  2. If they both match, then merge source into destination by giving the
 *  destination all of the memory allocated to source.
 *
 *  3. If destination is not already in the list, insert merged destination
 *  block into list by updating appropriate references.
 *
 * @param   dst     Destination block we are merging into.
 * @param   src     Source block we are merging from.
 * @return  Whether or not the merge completed successfully.
 **/
bool	block_merge(Block *dst, Block *src) {
    // TODO: Implement block merge
    
    if ((void*)(dst->data + dst->capacity) == (void*)src) {
        dst->capacity += sizeof(Block) + src->capacity;

        // If the destination is not already in the list
        // Alter the source pointers to accomodate the new block
        if (dst->next == dst) {
            src->prev->next = dst;
            dst->next = src->next;
            dst->prev = src->prev;
            src->next->prev = dst;
        }

        Counters[MERGES]++;
        Counters[BLOCKS]--;

        return true;
    }
    return false;
}

/**
 * Attempt to split block with the specified size:
 *
 *  1. Check if block capacity is sufficient for requested aligned size and
 *  header Block.
 *
 *  2. Split specified block into two blocks.
 *
 * @param   block   Pointer to block to split into two separate blocks.
 * @param   size    Desired size of the first block after split.
 * @return  Pointer to original block (regardless if it was split or not).
 **/
Block * block_split(Block *block, size_t size) {
    // TODO: Implement block split
    if (block->capacity > ALIGN(size) + sizeof(Block)) {

        // Create new block
        Block *split = (Block *)(block->data + ALIGN(size));

        // Update block after original block
        block->next->prev = split;

        // Update new block from split
        split->next = block->next;
        split->prev = block;
        split->capacity = block->capacity - (ALIGN(size) + sizeof(Block));
        split->size = split->capacity; 

        // Update original block being split
        block->next = split;
        block->capacity = ALIGN(size);
        block->size = size;

        // Update counters
        Counters[SPLITS]++;
        Counters[BLOCKS]++;

    } else {
        block->size = size;
    }
    return block;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
