#include "typedefs.h"

#ifndef _HEAP_MALLOC_H
#define _HEAP_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * heap_open
 *
 * Initialize the heap memory. This function depends on the required ZTE
 * heap library wrapper functions to be implemented.
 *
 */
void heap_open(void);

/**
 *
 * heap_malloc
 *
 * Implementation of a standard malloc() function to allocate memory.
 *
 * @param allocSize the size of memory in bytes to allocate
 * @return pointer to the allocated memory
 *
 */
void* heap_malloc(u32 alloc_size);

/**
 *
 * heap_free
 *
 * Implementation of a standard free() function to free allocated memory.
 *
 * @param beFree pointer to memory to free
 *
 */
void heap_free(void* be_free);

// settings functions for heap malloc
void* heap_settings_memoryBegin();

u32 heap_settings_memoryLimit();

u8 heap_settings_shared();

u32 heap_settings_getStackPointer();
//=========================================

#ifdef __cplusplus
}
#endif
#endif
