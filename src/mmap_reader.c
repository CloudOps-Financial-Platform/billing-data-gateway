#include "../include/mmap_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

bool mmap_open(const char* filepath, mmap_file_t* out_file) {
    if (!filepath || !out_file) return false;

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("[mmap] Failed to open file");
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("[mmap] Failed to stat file");
        close(fd);
        return false;
    }

    if (st.st_size == 0) {
        close(fd);
        return false;
    }

    void* mapped = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("[mmap] mmap failed");
        close(fd);
        return false;
    }

    madvise(mapped, st.st_size, MADV_SEQUENTIAL);

    out_file->data = (const char*)mapped;
    out_file->size = (size_t)st.st_size;
    out_file->fd = fd;
    return true;
}

void mmap_close(mmap_file_t* file) {
    if (file && file->data) {
        munmap((void*)file->data, file->size);
        close(file->fd);
        file->data = NULL;
        file->size = 0;
        file->fd = -1;
    }
}