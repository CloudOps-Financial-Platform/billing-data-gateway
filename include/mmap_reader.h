#ifndef MMAP_READER_H
#define MMAP_READER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char* data;
    size_t      size;
    int         fd;
} mmap_file_t;

bool mmap_open(const char* filepath, mmap_file_t* out_file);
void mmap_close(mmap_file_t* file);

#endif // MMAP_READER_H