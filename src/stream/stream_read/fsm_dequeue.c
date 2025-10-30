
/*============================ INCLUDES ======================================*/
#include ".\fsm_dequeue.h"
#include "perf_counter.h"


/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
def_simple_fsm(stream_read_flush,
    def_params(           
        stream_read_t  *ptStreamRead;
        mem_blk_t **pptByteFifo;
        mem_blk_t *ptByteFifo;
        dma_start_rx_fn *fnDmaStartRx;
        stream_read_flush_fn *fnFlush; 
    )
)

def_simple_fsm(dequeue,
    def_params(
        stream_read_t *ptStreamRead;
        mem_blk_t *ptByteFifo;
        dequeue_fn *fnDequeue;
    )
)

def_simple_fsm(time_out,
    def_params(
        stream_read_t *ptStreamRead;
        time_out_fn *fnTimeOut;
        fsm(stream_read_flush) fsmFlush;
    )
)



/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
static void tim_irq_trigger_delay_us(uint32_t wDelayTime) ;

//static void get_dma_cnt_and_dma_rx_init(stream_read_t *ptThis,mem_blk_t *ptByteFifo,
//    mem_blk_t **pptByteFifo);
static void get_dma_cnt(stream_read_t *ptThis,uint16_t *hwSize);

static bool is_equal_to_count_before_time_out(stream_read_t *ptThis);

static bool is_dma_busy(stream_read_t *ptThis);
static void set_dma_busy(stream_read_t *ptThis);


static bool is_uart_idle(stream_read_t *ptThis);
//void set_uart_busy(stream_read_t *ptThis);
//void set_uart_idle(stream_read_t *ptThis);

static bool is_really_time_out(stream_read_t *ptThis);
static uint16_t get_dma_data_cnt(stream_read_t *ptThis);
static void update_data_cnt(stream_read_t *ptThis,uint16_t hwDataCnt);

static bool is_timer_time_out(stream_read_t *ptThis);
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/




fsm_initialiser(dequeue,
    args(stream_read_t *ptStreamRead
    ))

    init_body(
        if ( NULL == ptStreamRead ) {
            abort_init();
        }
        this.ptByteFifo   = ptStreamRead->ptByteFifoEmpty;
        this.ptStreamRead = ptStreamRead;
        this.fnDequeue    = &dequeue;
    )

fsm_initialiser(stream_read_flush,
    args(stream_read_t *ptStreamRead ,mem_blk_t **pptByteFifo,dma_start_rx_fn *fnDmaStartRx
    ))

    init_body(  
        if (  NULL == ptStreamRead
            ||NULL == pptByteFifo
            ||NULL == fnDmaStartRx ) {
            abort_init();
        }

        this.ptStreamRead = ptStreamRead;
        this.pptByteFifo  = pptByteFifo;
        this.fnDmaStartRx = fnDmaStartRx;
        this.fnFlush      = &stream_read_flush;
    )

fsm_initialiser(time_out,
    args(stream_read_t *ptStreamRead 
    ))

    init_body(  
        if ( NULL == ptStreamRead) {
            abort_init();
        }

        this.ptStreamRead = ptStreamRead;
        this.fnTimeOut    = &time_out;
    )


        
        
implement_fsm(dequeue,
    args( byte *ptByte
    ))   
{
    def_states(BYTE_FIFO_OUT,GET_BYTE_FIFO) 

    body(    
        on_start(
            update_state_to(BYTE_FIFO_OUT);
        )
        state(BYTE_FIFO_OUT) {
             if (dequeue_byte_fifo(&(this.ptStreamRead->tByteFifo),ptByte) ) {  
                
                fsm_cpl();
             } else {
                block_free(&this.ptStreamRead->tMemBlockFifo,this.ptByteFifo);
                update_state_to(GET_BYTE_FIFO);               
             }
        }
        state(GET_BYTE_FIFO) {             
             mem_blk_t *ptByteFifo = block_fetch(&(this.ptStreamRead->tMemBlockFifo));   
             if (NULL == ptByteFifo) {
                 
             } else {
                 init_byte_fifo_full(&(this.ptStreamRead->tByteFifo),
                     ptByteFifo->chMemory,ptByteFifo->tSizeInByte);
                 this.ptByteFifo = ptByteFifo;
                
                 update_state_to(BYTE_FIFO_OUT);  
             }
        }
    )    
}


implement_fsm(stream_read_flush)   
{                 
    def_states(IS_DMA_BUSY,GET_BLOCK,GET_DATA_CNT) 
    
    body(    
        on_start(
            uint16_t hwSize = 0;
            update_state_to(IS_DMA_BUSY);
        ) 
    
        state(IS_DMA_BUSY) {
            bool bBusy = false;
            __IRQ_SAFE {
                bBusy = is_dma_busy(this.ptStreamRead);
            }
            
            if (false != bBusy) {
                fsm_cpl();
            } else {
                update_state_to(GET_BLOCK);
            }
                
        }
        
        state(GET_BLOCK) {
            this.ptByteFifo = block_new(&(this.ptStreamRead->tMemBlockFifo)); 
            if (NULL != this.ptByteFifo) { 
                this.ptByteFifo->tSizeInByte = this.ptStreamRead->hwDmaSizeTotal;
                update_state_to(GET_DATA_CNT);
            } else {

                update_state_to(GET_DATA_CNT);
            }
        }         
        
        state(GET_DATA_CNT) {
            uint16_t hwDataCnt = get_dma_data_cnt(this.ptStreamRead);
            if (0 == hwDataCnt) {                
                if (NULL != this.ptByteFifo) {
                    block_free(&this.ptStreamRead->tMemBlockFifo,this.ptByteFifo);
                }
                set_dma_idle(this.ptStreamRead);
                fsm_cpl();
            } else {
                update_data_cnt(this.ptStreamRead,hwDataCnt);                 
                block_append(&this.ptStreamRead->tMemBlockFifo,*this.pptByteFifo);
                if (NULL != this.ptByteFifo) {
                    (*this.ptStreamRead->fnDmaStartRx)(this.ptByteFifo);
                    this.ptStreamRead->ptByteFifoDmaRx = this.ptByteFifo;
                   
                }
                fsm_cpl();
            }

        }
        

    ) 
    
}

implement_fsm(time_out)
{                 
    def_states(IS_REALLY_TIME_OUT,FLUSH) 
   
    body(    
        on_start(  
            update_state_to(IS_REALLY_TIME_OUT);       
        )     
        state(IS_REALLY_TIME_OUT) { 
            if (is_really_time_out(this.ptStreamRead)) {   
                update_state_to(FLUSH);  
            } else {
                reset_fsm();
            }
        }


        state(FLUSH) {
            if (fsm_rt_cpl == call_fsm(stream_read_flush,&this.ptStreamRead->fsmTimeOut)) {
                fsm_cpl();
            }
        }
    )
}



static bool is_dma_busy(stream_read_t *ptThis)
{
    bool bRet = false;
    
    if (NULL == ptThis) {
        return false;
    }
    
    if (false == this.bBusy) {
        this.bBusy = true;
        bRet = false;
    } else {
        bRet = true;
    }
    
    return bRet;
}

static void set_dma_busy(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return ;
    }
    
    this.bBusy = true;
}

void set_dma_idle(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return ;
    }
    
    this.bBusy = false;
}

static bool is_really_time_out(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return false;
    }
    // uart busy
    if (false == is_timer_time_out(&this)) {
        return false;
    }
    if (false == is_uart_idle(&this)) {
        return false;
    }
    
    if (false == is_equal_to_count_before_time_out(&this)) {
        return false;
    }

    return true;
}



static bool is_uart_idle(stream_read_t *ptThis)
{
    bool bRet = false;
    if (NULL == ptThis) {
        return bRet;
    }
     
    if (false != this.bUartIdle) {
        bRet = true;
    }
    
    return bRet;
}

void set_uart_busy(stream_read_t *ptThis)
{
    
    if (NULL == ptThis) {
       return;
    }
     
    this.bUartIdle = false;
}

void set_uart_idle(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return ;
    }
    
    this.bUartIdle = true;
}

void record_current_data_count(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return;
    }
    
    this.hwDmaCntUartIdle = (*this.fnDmaCntGet)();
}

static bool is_equal_to_count_before_time_out(stream_read_t *ptThis)
{ 
    bool bRet = false;
    
    if (NULL == ptThis) {
        return false;
    }  
    
    uint16_t  hwCount = (*this.fnDmaCntGet)();
    
    if (this.hwDmaCntUartIdle == hwCount) {
        bRet = true;
    }
        
    return bRet;
}
static uint16_t get_dma_data_cnt(stream_read_t *ptThis)
{
    if (NULL == ptThis) {
        return false;
    }
    
    return (this.hwDmaSizeTotal - (*this.fnDmaCntGet)());
}

static void update_data_cnt(stream_read_t *ptThis,uint16_t hwDataCnt)
{
    if (NULL == ptThis
        ||0 == hwDataCnt ) {
        return;
    }
        
    this.ptByteFifoDmaRx->tSizeInByte = hwDataCnt;
}


void  set_target_time(stream_read_t *ptThis)
{
    if  (NULL == ptThis) {
        return;
    }
    
    uint32_t wTimeTarget = get_system_ms() + this.wSetTime;
    
    this.wTimeStamp  = wTimeTarget;
    this.bTimerStart = true;

}

static bool is_timer_time_out(stream_read_t *ptThis)
{
    bool bRet = false;
    
    if (NULL == ptThis) {
        return false;
    }
   
    if (false == this.bTimerStart) {
        return false;
    }

    if (this.wTimeStamp < get_system_ms()) {
        this.bTimerStart = false;
        bRet = true;
    }
    
    return bRet;
}




