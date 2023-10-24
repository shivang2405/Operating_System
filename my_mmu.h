#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define BLOCK_SIZE 1024

// Structure to represent memory blocks
struct Block
{
    size_t size;
    struct Block *next;
    int free;
};

static struct Block *free_list = NULL;

// Function to allocate memory using mmap
void *my_malloc(size_t size)
{
    size_t block_size = size + sizeof(struct Block);
    struct Block *block = (struct Block *)mmap(NULL, block_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED)
    {
        perror("Error in mmap");
        return NULL;
    }
    block->size = size;
    block->free = 0;
    block->next = free_list;
    free_list = block;
    return (void *)(block + 1);
}

// Function to allocate and initialize memory to zero using mmap
void *my_calloc(size_t nelem, size_t size)
{
    size_t block_size = nelem * size;
    void *ptr = my_malloc(block_size);
    if (ptr)
    {
        memset(ptr, 0, block_size);
    }
    return ptr;
}

// Function to free memory using munmap
void my_free(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    struct Block *block = (struct Block *)((char *)ptr - sizeof(struct Block));

    if (block != free_list)
    {
        struct Block *current = free_list;
        while (current && current->next != block)
        {
            current = current->next;
        }
        if (current)
        {
            current->next = block->next;
        }
    }
    else
    {
        free_list = free_list->next;
    }

    if (munmap(block, block->size + sizeof(struct Block)) == -1)
    {
        perror("Error in munmap");
    }
}

// Function to debug and print the current state of the heap
void debug()
{
    struct Block *current = free_list;
    size_t totalSize = 0;
    size_t blockCount = 0;

    fprintf(stderr, "Current state of the heap:\n");
    while (current)
    {
        fprintf(stderr, "Block %zu: %s, Size: %zu\n", blockCount, current->free ? "Free" : "Allocated", current->size);
        totalSize += current->size;
        blockCount++;
        current = current->next;
    }
    fprintf(stderr, "Total Blocks: %zu, Total Size: %zu\n", blockCount, totalSize);
} 