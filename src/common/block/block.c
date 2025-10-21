
#include "block.h"


#undef  this
#define this    (*ptThis)


void block_fifo_init(mem_blk_fifo_t *ptThis)  
{   
    if ( NULL == ptThis ) {
        return;
    }
    
    memset(&this,0,sizeof(mem_blk_fifo_t));  
}

void block_free(mem_blk_fifo_t *ptThis,mem_blk_t *ptFreeBlock) 
{   
    if ( NULL == ptThis ) {
        return ;
    } 
    if ( NULL == ptFreeBlock ) {
        return ;
    }
   
    ptFreeBlock->ptNext = this.ptFreeList; 
    this.ptFreeList = ptFreeBlock; 
}

mem_blk_t *block_new(mem_blk_fifo_t *ptThis) 
{     
    if ( NULL == ptThis ) {
        return NULL;
    }
    mem_blk_t *ptTemp = this.ptFreeList;  
    if ( NULL == ptTemp ) {
        return NULL;
    }
    
    this.ptFreeList = this.ptFreeList->ptNext ; 
    
    return ptTemp;
}

bool block_append(mem_blk_fifo_t *ptThis,mem_blk_t *ptNewNode) 
{
    if ( NULL == ptNewNode ) {
        return false;
    }
    if ( NULL == ptThis ) {
        return false;
    }
    ptNewNode->ptNext = NULL;
    /*   fifo empty   */
    if ( NULL == this.FIFO.ptTail ) {        
        this.FIFO.ptTail = ptNewNode;
        this.FIFO.ptHead = this.FIFO.ptTail;
    } else {  
        /* update  link */
        this.FIFO.ptTail->ptNext = ptNewNode;
        /* update  node */
        this.FIFO.ptTail = ptNewNode;  
    } 
   
    return true;
}

mem_blk_t *block_fetch(mem_blk_fifo_t *ptThis) 
{
    if ( NULL == ptThis ) {
        return NULL;
    }
    mem_blk_t *ptTemp = this.FIFO.ptHead;
    if ( NULL == ptTemp ) {
        return NULL;
    }
    
    /* the last one block */
    if ( this.FIFO.ptHead == this.FIFO.ptTail ) {
        this.FIFO.ptTail = NULL;
    }

    this.FIFO.ptHead = this.FIFO.ptHead->ptNext;  
    /* user get target from ptTemp,
    and can know module inside information by ptTemp->ptNext
    so we would best to set ptNext 0 for safety */
    ptTemp->ptNext = NULL;

    return ptTemp;
}