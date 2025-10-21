
#include ".\stream_write.h"

extern_fsm_initialiser(enqueue,
    args(stream_write_t *ptStreamWrite
    ));

extern_fsm_initialiser(stream_write_flush,
    args(stream_write_t *ptStreamWrite,mem_blk_t **pptByteFifo,dma_start_fn *fnDmaStart
    ))



void stream_write_init(stream_write_t *ptThis,stream_write_cfg_t *ptCfg)
{
    if (NULL == ptThis 
        || NULL == ptCfg) {
        return;
    }
    
    write_fifo_t *ptAddr = ptCfg->pchBuffer;
    uint8_t chCnt        = ptCfg->hwSize/sizeof(write_fifo_t);
    memset(&this,0,sizeof(stream_write_t));
     
    block_fifo_init(&(this.tMemBlockFifo)); 
    for(uint8_t chIndex = 0;chIndex < chCnt;chIndex++) {
        ptAddr[chIndex].tList.tSizeInByte = WRITE_BUFF_CNT;
        block_free(&(this.tMemBlockFifo),&ptAddr[chIndex].tList);
    }
     
    this.bBusy = false;
    this.ptByteFifoCpl = NULL;
    this.fnDmaStart = ptCfg->fnDmaSendData;
    
    this.ptByteFifo = block_new(&this.tMemBlockFifo);
    init_byte_fifo_empty(&(this.tByteFifo),this.ptByteFifo->chMemory,this.ptByteFifo->tSizeInByte);    
    this.ptByteFifo->tSizeInByte = 0;
    
    init_fsm(enqueue,&(this.fsmEnqueue),
        args(&this
    ));

    init_fsm(stream_write_flush,&(this.fsmFlushAuto),
        args(&this,              
            &(this.ptByteFifoCpl), 
            this.fnDmaStart   
    )); 
            
    init_fsm(stream_write_flush,&(this.fsmFlushManual),
        args(&this,            
            &(this.ptByteFifo),      
            this.fnDmaStart
    ));            
}



bool stream_write(stream_write_t *ptThis,byte tByte)
{
    bool bRet = false;
    if ( NULL == ptThis ) {
        return bRet;
    }
    
    if ( fsm_rt_cpl == call_fsm(enqueue,&(this.fsmEnqueue),args(tByte) ) ) {
        bRet = true;
    }
    
    return bRet;
}

void dma_send_data_cpl_event_handler(stream_write_t *ptThis)
{
    if (NULL == ptThis) {
        return ;
    }
        
    block_free(&this.tMemBlockFifo,this.ptFifoSend);  
    this.bBusy = false; 
    
    call_fsm(stream_write_flush,&(this.fsmFlushAuto));           
}
