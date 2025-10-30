
#include ".\stream_read.h"
#include ".\fsm_dequeue.h"


extern_fsm_initialiser(dequeue,
    args(stream_read_t *ptStreamRead
    )) ;

extern_fsm_initialiser(stream_read_flush,
    args(stream_read_t *ptStreamRead,
    mem_blk_t **pptByteFifo,
    dma_start_rx_fn *fnDmaStartRx
    )) ;

extern_fsm_initialiser(time_out,
    args(stream_read_t *ptStreamRead
    )) ;


void stream_read_init(stream_read_t *ptThis,stream_read_cfg_t *ptCfg )
{
    if (   NULL == ptThis 
        || NULL == ptCfg ) {
        return ;
    }
        
    read_fifo_t *ptAddr = ptCfg->pchBuffer;
    uint8_t chCnt       = ptCfg->hwSize/sizeof(read_fifo_t);
    memset(&this,0,sizeof(stream_read_t));

    block_fifo_init(&(this.tMemBlockFifo)); 
    for(uint8_t chIndex = 0;chIndex < chCnt;chIndex++) {
        ptAddr[chIndex].tList.tSizeInByte = READ_BUFF_CNT;
        block_free(&(this.tMemBlockFifo),&ptAddr[chIndex].tList);
    }
    //this.fnRead = stream_read;
    this.hwDmaSizeTotal = READ_BUFF_CNT;
    this.fnDmaCntGet = ptCfg->fnDmaCntGet ;
    this.bBusy = false;
    this.bUartIdle = false;
    this.bTimerStart = false;
    this.wSetTime = ptCfg->wTimeOutMs;        
    this.fnDmaStartRx = ptCfg->fnDmaStartRx;
    //this.fnTimeTrigger = ptCfg->fnTimeTrigger;
    this.fnGetTimeStamp = ptCfg->fnGetTimeStamp;
    this.ptByteFifoDmaRx  = block_new(&(this.tMemBlockFifo));
    (*this.fnDmaStartRx)(this.ptByteFifoDmaRx);
    
    this.ptByteFifoEmpty  = block_new(&(this.tMemBlockFifo));
    init_byte_fifo_empty(&(this.tByteFifo),this.ptByteFifoEmpty->chMemory,
        this.ptByteFifoEmpty->tSizeInByte);
   
    
    init_fsm(stream_read_flush,&(this.fsmFlushHt),
        args(&this,&this.ptByteFifoDmaRx,this.fnDmaStartRx) );
    init_fsm(dequeue,&(this.fsmDequeue),args(&this));
    init_fsm(time_out,&(this.fsmTimeOut),args(&this));
    
    init_fsm(stream_read_flush,&this.fsmTimeOut,
        args(&this,&this.ptByteFifoDmaRx,this.fnDmaStartRx));

}


bool stream_read(stream_read_t *ptThis,byte *pchChar) 
{
    bool bRet = false ;
    if (   NULL == ptThis 
        || NULL == pchChar ) {
        return bRet;
    }

    if ( fsm_rt_cpl == call_fsm(dequeue,&this.fsmDequeue,args(pchChar) ) ) {
        bRet = true;        
    }

    return bRet;
}

void uart_dma_get_data_insert_to_dma_irq_event_handler(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return ;
    }
    
    set_dma_idle(&this);
    set_uart_busy(&this);
    call_fsm(stream_read_flush,&this.fsmFlushHt);
}


void uart_idle_insert_to_uart_irq_event_handler(stream_read_t *ptThis)
{
    if ( NULL == ptThis ) {
        return;
    }
    
    set_uart_idle(&this);
    record_current_data_count(&this);
    set_target_time(&this);
}

void uart_wait_time_out_insert_to_hard_timer_irq_event_handler(stream_read_t *ptThis) 
{
    if ( NULL == ptThis ) {
        return;
    }
   
    call_fsm(time_out,&(this.fsmTimeOut)); 
    
    

}



