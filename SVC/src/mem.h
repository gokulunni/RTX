#include "rtx.h"

int mem_init(size_t blk_size, int algo);
void *mem_alloc(size_t size);
void mem_dealloc(void *ptr);
int mem_count_extfrag(size_t size);
