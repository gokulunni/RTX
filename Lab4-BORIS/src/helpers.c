//
// Created by Boris Nguyen on 2020-11-22.
//

#include "k_mem.h"
#include "common.h"

int mem_cpy(void *destination, void *source, size_t size) {
    if (destination == NULL || source == NULL) {
        return RTX_ERR;
    }

    if (size <= 0) {
        return RTX_ERR;
    }

    char *dest = destination;
    char *src = source;
    for (int i = 0; i < size; i++) {
        dest[i] = src[i];
    }

    return RTX_OK;
}