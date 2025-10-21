#ifndef _BLOCK_H
#define _BLOCK_H

#include ".\app_cfg.h" 

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct mem_blk_t {
    struct mem_blk_t *ptNext;
    size_t tSizeInByte;
    uint8_t chMemory[];
} mem_blk_t;

typedef struct mem_blk_fifo_t {
    mem_blk_t *ptFreeList;
    struct {
        mem_blk_t *ptHead;
        mem_blk_t *ptTail;
    } FIFO;
} mem_blk_fifo_t;

extern void block_fifo_init(mem_blk_fifo_t *ptThis) ;
extern void block_free(mem_blk_fifo_t *ptThis,mem_blk_t *ptFreeBlock);
extern mem_blk_t *block_new(mem_blk_fifo_t *ptThis) ;
extern bool block_append(mem_blk_fifo_t *ptThis, mem_blk_t *ptNewNode) ;
extern mem_blk_t *block_fetch(mem_blk_fifo_t *ptThis) ;

#endif

