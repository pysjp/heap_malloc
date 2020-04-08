
#include "heap_malloc.h"

u8 heap_stack[64 * 1024];

void* heap_settings_memoryBegin() {
    return heap_stack;
}

u32 heap_settings_memoryLimit() {
    return 64 * 1024;
}
u8 heap_settings_shared() {
    return 0;
}

u32 heap_settings_getStackPointer() {
    return 0;
}