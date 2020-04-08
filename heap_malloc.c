

#include "heap_malloc.h"

/* This structure must be 4-byte aligned. */
typedef struct {
    volatile u8* next;
    volatile u8* previous;
    u32 size;
} heap_record_t;

#ifdef _HEAP_64
#define ptr_size u64
#else
#define ptr_size u32
#endif
#define mem_ptr ptr_size*

#define HEAP_REC_SIZE sizeof(heap_record_t)
#define HEAP_PICKET_SIZE sizeof(ptr_size)
#define HEAP_PICKET_VALUE 0x00464200 /* "\0BF\0" */
#define HEAP_PICKET_RESET 0xffffffff

/**
 *
 * heap_open
 *
 * Initialize the heap memory. This function depends on the required ZTE
 * heap library wrapper functions to be implemented.
 *
 */
void heap_open(void) {
    /* Variable Declarations */
    volatile mem_ptr picket;
    volatile heap_record_t* heapRec;
    ptr_size heap_ulHeapMemoryBegin;
    volatile u8* heap_heapBegin;

    /* Acquire numbers from the wrapper functions. */
    heap_ulHeapMemoryBegin = (ptr_size)heap_settings_memoryBegin();
    heap_heapBegin = (volatile u8*)heap_ulHeapMemoryBegin;
    /* Put a dummy record to memory. */
    picket = (mem_ptr)heap_heapBegin;
    *picket = HEAP_PICKET_VALUE;
    heapRec = (heap_record_t*)(heap_heapBegin + HEAP_PICKET_SIZE);
    heapRec->next = 0;
    heapRec->previous = 0;
    heapRec->size = 0;
    picket = (mem_ptr)(heap_heapBegin + HEAP_PICKET_SIZE + HEAP_REC_SIZE);
    *picket = HEAP_PICKET_VALUE;
}

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
void* heap_malloc(u32 allocSize) {
    /* Variable Declarations */
    volatile char* retPointer;
    ptr_size allocSize4;
    volatile heap_record_t* heapRec;
    volatile heap_record_t* heapRecNew;
    volatile heap_record_t* heapRecNext;
    volatile heap_record_t* lastBestFitHeapRec;
    ptr_size lastBestFitRate;
    ptr_size currentFitRate;
    volatile mem_ptr picket;
    ptr_size freeSize;
    ptr_size requiredSize;
    ptr_size currentHeapMemoryTop;
    ptr_size heap_ulHeapMemoryLimit;
    u8 heap_bHeapMemorySPShared;
    ptr_size heap_ulHeapMemoryBegin;
    volatile u8* heap_heapBegin;

    /* Initialize */
    heap_ulHeapMemoryLimit = heap_settings_memoryLimit();
    heap_bHeapMemorySPShared = heap_settings_shared();
    heap_ulHeapMemoryBegin = (ptr_size)heap_settings_memoryBegin();
    heap_heapBegin = (volatile u8*)heap_ulHeapMemoryBegin;
    /* Basic checking. */
    if (allocSize == 0) {
        return (NULL_POINTER);
    }

    /* Allocate size must be 4-byte alignment. */
    allocSize4 = allocSize - (allocSize & 0x3);
    if (allocSize4 < allocSize) {
        allocSize4 += 4;
    }

    /* Prepare for walking. */
    heapRec = (heap_record_t*)(heap_heapBegin + HEAP_PICKET_SIZE);
    requiredSize = allocSize4 + 2 * HEAP_PICKET_SIZE + HEAP_REC_SIZE;
    lastBestFitHeapRec = NULL_POINTER;
    lastBestFitRate = 0xfffffffe;

    /* Walking. */
    do {
        /* Check if heapRec is located at the correct memory range. */
        if (heap_bHeapMemorySPShared) {
            /* The heap memory pool is shared with stack memory so get the */
            /* current stack pointer. */
            currentHeapMemoryTop = heap_settings_getStackPointer();

            /* Put the space reserved for user stack. */
            currentHeapMemoryTop -= heap_ulHeapMemoryLimit;
        } else {
            /* The heap memory pool is dedicated so get the top memory pointer
             */
            currentHeapMemoryTop =
                heap_ulHeapMemoryBegin + heap_ulHeapMemoryLimit;
        }

        if (((ptr_size)heapRec) <
            (((ptr_size)heap_ulHeapMemoryBegin) + HEAP_PICKET_SIZE)) {
            return (NULL_POINTER);
        }

        if ((((ptr_size)heapRec) + HEAP_REC_SIZE + HEAP_PICKET_SIZE) >=
            currentHeapMemoryTop) {
            return (NULL_POINTER);
        }

        /* Check picket for access violation check. */
        picket = (mem_ptr)(((volatile u8*)heapRec) - HEAP_PICKET_SIZE);
        if (*picket != HEAP_PICKET_VALUE) {
            return (NULL_POINTER);
        }

        picket =
            (mem_ptr)(((volatile u8*)heapRec) + HEAP_REC_SIZE + heapRec->size);
        if (*picket != HEAP_PICKET_VALUE) {
            return (NULL_POINTER);
        }

        /* Find the size of the next free block. */
        if (((ptr_size)(heapRec->next)) > 0) {
            freeSize = ((ptr_size)(heapRec->next)) - ((ptr_size)heapRec) -
                       2 * HEAP_PICKET_SIZE - HEAP_REC_SIZE - heapRec->size;
        } else {
            /* Get the available size for a new block. */
            freeSize = ((ptr_size)(currentHeapMemoryTop)) -
                       ((ptr_size)heapRec) - HEAP_PICKET_SIZE - HEAP_REC_SIZE -
                       heapRec->size;
        }

        /* Go for best fit. */
        if (requiredSize == freeSize) {
            /* Perfect match. */
            lastBestFitHeapRec = heapRec;

            /* Stop walking. */
            break;
        } else if (requiredSize < freeSize) {
            /* Check fit rate. */
            currentFitRate = freeSize - requiredSize;
            if (currentFitRate < lastBestFitRate) {
                lastBestFitHeapRec = heapRec;
                lastBestFitRate = currentFitRate;
            }
        }

        heapRec = (heap_record_t*)heapRec->next;

    } while (((ptr_size)heapRec) > 0);

    /* Allocate the new block. */
    if (lastBestFitHeapRec == NULL_POINTER) {
        retPointer = NULL_POINTER;
    } else {
        heapRec = lastBestFitHeapRec;
        heapRecNext = (heap_record_t*)heapRec->next;
        heapRecNew = (heap_record_t*)(((volatile u8*)heapRec) + HEAP_REC_SIZE +
                                      heapRec->size + 2 * HEAP_PICKET_SIZE);

        /* Set picket. */
        picket = (mem_ptr)(((volatile u8*)heapRecNew) - HEAP_PICKET_SIZE);
        *picket = HEAP_PICKET_VALUE;

        heapRecNew->next = (u8*)heapRecNext;
        heapRecNew->previous = (u8*)heapRec;
        heapRecNew->size = allocSize4;

        picket =
            (mem_ptr)(((volatile u8*)heapRecNew) + HEAP_REC_SIZE + allocSize4);
        *picket = HEAP_PICKET_VALUE;

        /* Link heap record. */
        heapRec->next = (u8*)heapRecNew;
        if (((ptr_size)heapRecNext) > 0) {
            heapRecNext->previous = (u8*)heapRecNew;
        }

        /* Set buffer pointer. */
        retPointer = ((volatile char*)heapRecNew) + HEAP_REC_SIZE;
    }

    return ((void*)retPointer);
}

/**
 *
 * heap_free
 *
 * Implementation of a standard free() function to free allocated memory.
 *
 * @param beFree pointer to memory to free
 *
 */
void heap_free(void* beFree) {
    /* Variable Declarations */
    volatile heap_record_t* heapRec;
    volatile heap_record_t* heapRecNext;
    volatile heap_record_t* heapRecPrevious;
    volatile mem_ptr picket1;
    volatile mem_ptr picket2;
    ptr_size heap_ulHeapMemoryBegin = (ptr_size)heap_settings_memoryBegin();

    /* beFree basic checking. Return if incorrect. */
    if (((ptr_size)beFree) < heap_ulHeapMemoryBegin) {
        return;
    }

    /* Addressing the memory location. */
    heapRec = ((heap_record_t*)(((volatile u8*)beFree) - HEAP_REC_SIZE));
    picket1 = (mem_ptr)(((volatile u8*)heapRec) - HEAP_PICKET_SIZE);
    picket2 =
        (mem_ptr)(((volatile u8*)heapRec) + HEAP_REC_SIZE + heapRec->size);

    /* Check picket first, if not match then do nothing. */
    if (*picket1 != HEAP_PICKET_VALUE) {
        return;
    }

    if (*picket2 != HEAP_PICKET_VALUE) {
        return;
    }

    /* Set record first. */
    heapRecPrevious = (heap_record_t*)heapRec->previous;
    heapRecNext = (heap_record_t*)heapRec->next;

    /* Reset picket value. */
    *picket1 = HEAP_PICKET_RESET;
    *picket2 = HEAP_PICKET_RESET;

    /* Relink record. */
    heapRecPrevious->next = (u8*)heapRecNext;
    if (((ptr_size)heapRecNext) > 0) {
        heapRecNext->previous = (u8*)heapRecPrevious;
    }

    /* Reset heap record. */
    heapRec->next = 0;
    heapRec->previous = 0;
    heapRec->size = 0;
}
