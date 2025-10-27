
#include ".\fsm_enqueue.h"
#include "perf_counter.h"

def_simple_fsm(stream_write_flush,
    def_params(
        stream_write_t *ptStreamWrite;
        mem_blk_t **pptByteFifo;
        void (*fnDmaStart)(mem_blk_t *ptFifoSend); 
        stream_write_flush_fn  *fnFlush ;
    )
)


def_simple_fsm(enqueue,
    def_params(          
        fsm(stream_write_flush) fsmFlush;
        stream_write_t *ptStreamWrite;
        enqueue_fn *fnEnqueue;
    )
)

fsm_initialiser(enqueue,
    args(stream_write_t *ptStreamWrite
    ))

    init_body (
        if ( NULL == ptStreamWrite ) {
            abort_init();
        }
        
        this.ptStreamWrite  = ptStreamWrite;      
        this.fnEnqueue      = &enqueue;         
    )

fsm_initialiser(stream_write_flush,
    args(stream_write_t *ptStreamWrite,mem_blk_t **pptByteFifo,dma_start_fn *fnDmaStart
    ))
    
    init_body (  
        if ( NULL == ptStreamWrite            
            ||NULL == pptByteFifo
            ||NULL == fnDmaStart
        ) {
            abort_init();
        }
        
        this.ptStreamWrite      = ptStreamWrite;
        this.pptByteFifo   = pptByteFifo;
        this.fnDmaStart    = fnDmaStart;
        this.fnFlush       = &stream_write_flush;
    )

implement_fsm(enqueue,
    args ( byte tByte
    ))   
{     

    def_states(BYTE_FIFO_IN,FLUSH,GET_BYTE_FIFO) 
    
    body(    
        on_start(                    
            update_state_to(BYTE_FIFO_IN);
        )                    
        state(BYTE_FIFO_IN) {            
             if (enqueue_byte_fifo(&(this.ptStreamWrite->tByteFifo),tByte) ) {  
                this.ptStreamWrite->ptByteFifo->tSizeInByte++;               
                fsm_cpl();
             } else {
                init_fsm(stream_write_flush ,&(this.fsmFlush),
                    args(this.ptStreamWrite,                                             
                    &(this.ptStreamWrite->ptByteFifo),     
                    this.ptStreamWrite->fnDmaStart
                ));                                 
                update_state_to(FLUSH);               
             }            
        }        
        state(FLUSH) {   
            if (fsm_rt_cpl == call_fsm(stream_write_flush,&(this.fsmFlush) ) ) {                               
                update_state_to(GET_BYTE_FIFO); 
            }                
        } 
        state(GET_BYTE_FIFO) {             
            mem_blk_t *ptByteFifo = block_new(&this.ptStreamWrite->tMemBlockFifo);                       
            if (NULL != ptByteFifo) {
                init_byte_fifo_empty(&(this.ptStreamWrite->tByteFifo),ptByteFifo->chMemory,ptByteFifo->tSizeInByte); 
                ptByteFifo->tSizeInByte = 0;
                this.ptStreamWrite->ptByteFifo = ptByteFifo;                 
                update_state_to(BYTE_FIFO_IN);               
            }                                              
        }        
    )    
}


implement_fsm(stream_write_flush)   
{                 
    def_states(IS_DMA_BUSY,GET_BLOCK) 
    // body_begin();  
    body(    
        on_start(                                       
            block_append(&this.ptStreamWrite->tMemBlockFifo,*(this.pptByteFifo) );                                                       
            update_state_to(IS_DMA_BUSY);                            
        )
    
        state(IS_DMA_BUSY) { 
            bool bBusy = false;
            
            __IRQ_SAFE {
                bBusy = (false == this.ptStreamWrite->bBusy)?((this.ptStreamWrite->bBusy = true),false):(true);
            }
            
            if (false == bBusy) {          
                update_state_to(GET_BLOCK);           
            } else {
                fsm_cpl();
            }                    
        }         
        state(GET_BLOCK) {                            
            this.ptStreamWrite->ptFifoSend = block_fetch(&this.ptStreamWrite->tMemBlockFifo);           
            if (NULL == this.ptStreamWrite->ptFifoSend ) {  
                this.ptStreamWrite->bBusy = false;
                fsm_cpl();
            } else {                            
                (*this.fnDmaStart)(this.ptStreamWrite->ptFifoSend); 
                fsm_cpl();               
            }        
        } 
    )  
       // body_end();      
}






        


