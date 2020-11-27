//
// Created by Boris Nguyen on 2020-11-22.
//

#include "helpers.h"
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

int str_cmp(const char *str1, const char *str2) {
    int s1;
    int s2;
    do {
        s1 = *str1++;
        s2 = *str2++;
        if (s1 == 0)
            break;
    } while (s1 == s2);
    return (s1 != s2) ? RTX_ERR : RTX_OK;
}