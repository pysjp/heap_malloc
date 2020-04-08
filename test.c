
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "heap_malloc.h"

int main() {
    heap_open();
    void* p = heap_malloc(1024);
    assert(p != NULL);
    strcpy(p, "heap malloc");
    assert(strcmp(p, "heap malloc") == 0);
    // do someing
    heap_free(p);

    // large out of range
    p = heap_malloc(64 * 1024);
    assert(p == NULL);
    return 0;
}