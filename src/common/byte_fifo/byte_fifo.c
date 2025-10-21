
#include "byte_fifo.h"


#undef  this
#define this    (*ptThis)

bool init_byte_fifo_empty(byte_fifo_t *ptThis,byte *ptBuff,uint16_t hwSize)
{
    bool bRet = false;
    
    if (    (NULL == ptThis) 
       ||   (NULL == ptBuff)  
       ||   (0 == hwSize)) {
        return bRet;
    }
       
    this.pchBuffer = ptBuff;
    this.hwSize    = hwSize;
    this.hwHead    = 0;
    this.hwTail    = 0;
    this.hwLength  = 0;
    
    bRet = true;
    return bRet;
}

bool init_byte_fifo_full(byte_fifo_t *ptThis,byte *ptBuff,uint16_t hwSize)
{
    bool bRet = false;
    
    if (    (NULL == ptThis)  
       ||   (NULL == ptBuff)  
       ||   (0 == hwSize)) {
        return bRet;
    }
         
    this.pchBuffer = ptBuff;
    this.hwSize    = hwSize;
    this.hwHead    = 0;
    this.hwTail    = 0;
    this.hwLength  = hwSize;
    
    bRet = true;
    return bRet;
}





bool enqueue_byte_fifo(byte_fifo_t *ptThis,byte tByteToIn)
{
    bool bRet = false;
    
    if (NULL == ptThis) {
        return bRet;
    }
    /* fifo full */
    if (    (this.hwHead == this.hwTail) 
       &&   (0 != this.hwLength)) {
        return  bRet; 
    }
    
    this.pchBuffer[this.hwTail++] = tByteToIn;
    
    if (this.hwTail >= this.hwSize) {
        this.hwTail = 0;
    }
    this.hwLength++;
    bRet = true;
    return bRet;
}

bool dequeue_byte_fifo(byte_fifo_t *ptThis,byte *ptToOut)
{
    bool bRet = false;
    
    if (    (NULL == ptThis) 
       ||   (NULL == ptToOut)) {
        return bRet;
    }    
    /* fifo empty */
    if (    (this.hwHead == this.hwTail) 
       &&   (0 == this.hwLength)) {
        return bRet;   
    }
    
    *ptToOut = this.pchBuffer[this.hwHead++];
    
    if (this.hwHead >= this.hwSize) {
        this.hwHead = 0;
    }
    this.hwLength--;
    bRet = true;
    return bRet;
}

bool is_byte_fifo_empty(byte_fifo_t *ptThis)
{
    bool bRet = false;
    
    if (NULL == ptThis) {
        return bRet;
    }
    
    if (    (this.hwHead == this.hwTail) 
       &&   (0 == this.hwLength)) {
        bRet  = true;   
    }
    return bRet;
}




