#ifndef __STREAM_READ__
#define __STREAM_READ__

//#include ".\__common.h"

#include "..\..\common\common.h"
#include "..\stream_cfg.h"

typedef struct stream_read_t stream_read_t;
typedef void(dma_start_rx_fn)(mem_blk_t *ptFifoSend);
typedef uint16_t(dma_cnt_get_fn)(void);
typedef void (time_trigger_fn)(uint32_t wTimeOut);
typedef union {
    mem_blk_t tList;
    uint8_t buffer[sizeof(mem_blk_t) + READ_BUFF_CNT];
} read_fifo_t;

declare_simple_fsm(dequeue);
declare_simple_fsm(stream_read_flush);
declare_simple_fsm(time_out);

extern_fsm_implementation(dequeue,
    args(byte *ptByte
    )) ;

extern_fsm_implementation(stream_read_flush); 

extern_fsm_implementation(time_out); 


extern_simple_fsm(stream_read_flush,
    def_params(           
        stream_read_t  *ptStreamRead; 
        mem_blk_t **pptByteFifo;
        mem_blk_t *ptByteFifo;
        dma_start_rx_fn *fnDmaStartRx;
        stream_read_flush_fn  *fnFlush;
    )
)

extern_simple_fsm(dequeue,
    def_params(          
        stream_read_t *ptStreamRead;
        mem_blk_t *ptByteFifo;
        dequeue_fn *fnDequeue;
    )
)

extern_simple_fsm(time_out,
    def_params(          
        stream_read_t *ptStreamRead;
        time_out_fn *fnTimeOut;
        fsm(stream_read_flush) fsmFlush;
    )
)

struct stream_read_t {
    bool bBusy ;
    bool bUartIdle;
    mem_blk_fifo_t tMemBlockFifo ;   
    byte_fifo_t tByteFifo ; 
    mem_blk_t *ptByteFifoDmaRx ; 
    mem_blk_t *ptByteFifoEmpty ;     
    uint16_t hwDmaCntUartIdle;
    uint16_t hwDmaCntTimeOut;
    uint32_t wTargertTime;
    dma_start_rx_fn *fnDmaStartRx;
    dma_cnt_get_fn *fnDmaCntGet;
    time_trigger_fn *fnTimeTrigger;
    fsm(stream_read_flush) fsmFlushHt ;  
    fsm(dequeue) fsmDequeue ;
    fsm(time_out)  fsmTimeOut ;
    uint16_t hwDmaSizeTotal;
} ;



typedef struct  {
    uint8_t *pchBuffer;
    uint16_t hwSize;
    uint32_t wTimeOutMs;
    dma_start_rx_fn *fnDmaStartRx;
    dma_cnt_get_fn *fnDmaCntGet;
    time_trigger_fn *fnTimeTrigger;
} stream_read_cfg_t;



extern
void stream_read_init(stream_read_t *ptThis,stream_read_cfg_t *ptCfg ) ;
extern
void uart_dma_get_data_insert_to_dma_irq_event_handler(stream_read_t *ptThis) ;
extern
void uart_idle_insert_to_uart_irq_event_handler(stream_read_t  *ptThis) ;
extern
bool stream_read(stream_read_t *ptThis,byte *pchChar) ;
extern
void uart_wait_time_out_insert_to_hard_timer_irq_event_handler(stream_read_t *ptThis) ;
#endif

