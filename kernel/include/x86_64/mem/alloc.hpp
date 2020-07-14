#ifndef FREELSD_KERNEL_MEMORY_LIBALLOC_HEADER
#define FREELSD_KERNEL_MEMORY_LIBALLOC_HEADER

#include <stdint.h>
#include <stddef.h>

#define LIBALLOC_KERNEL_HEAP_START 0xFFFFFFFFC0000000
#define LIBALLOC_KERNEL_HEAP_FINAL 0xFFFFFFFFFFFFFFFF

// A structure found at the top of all system allocated
// memory blocks. It details the usage of the memory block.
typedef struct liballoc_major {
    struct liballoc_major *prev;   // Linked list information.
    struct liballoc_major *next;   // Linked list information.
    unsigned int pages;            // The number of pages in the block.
    unsigned int size;             // The number of pages in the block.
    unsigned int usage;            // The number of bytes used in the block.
    struct liballoc_minor *first;  // A pointer to the first allocated memory in the block.
} liballoc_major_t;

// This is a structure found at the beginning of all
// sections in a major block which were allocated by a
// malloc, calloc or realloc call.
typedef struct liballoc_minor {
    struct liballoc_minor *prev;    // Linked list information.
    struct liballoc_minor *next;    // Linked list information.
    struct liballoc_major *block;   // The owning block. A pointer to the major structure.
    unsigned int magic;             // A magic number to identify correctness.
    unsigned int size;              // The size of the memory allocated. Could be one byte or more.
    unsigned int reqsize;           // The size of memory requested.
} liballoc_minor_t;

extern uint64_t liballoc_warnings;  // The number of liballoc warnings generated.
extern uint64_t liballoc_errorcnt;  // The number of liballoc errors generated.
extern uint64_t liballoc_poverrun;  // The number of potential liballoc overruns.

// Kernel malloc. Allocates memory on the kernel heap.
void *kmalloc(size_t reqsize);

// Kernel realloc. Allocates memory on the kernel heap.
void *krealloc(void *pointer, size_t size);

// Kernel calloc. Allocates memory on the kernel heap.
void *kcalloc(size_t objcount, size_t objsize);

// Kernel free.
void kfree(void *pointer);

#endif
