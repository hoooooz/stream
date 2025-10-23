
#ifndef __STREAM_WRITE__
#define __STREAM_WRITE__


#include "..\..\common\common.h"
#include "..\stream_cfg.h"

typedef union {
    mem_blk_t tList;
    uint8_t buffer[sizeof(mem_blk_t) + WRITE_BUFF_CNT];
} write_fifo_t;


typedef struct stream_write_t stream_write_t;


declare_simple_fsm(enqueue);
declare_simple_fsm(stream_write_flush);

extern_fsm_implementation(enqueue,
    args(byte tByte
    )) ;

extern_fsm_implementation(stream_write_flush); 



extern_simple_fsm(stream_write_flush,
    def_params(
        stream_write_t *ptStreamWrite;
        mem_blk_t **pptByteFifo;
        void (*fnDmaStart)(mem_blk_t *ptFifoSend); 
        stream_write_flush_fn  *fnFlush ;
    )
)

extern_simple_fsm(enqueue,
    def_params(
        fsm(stream_write_flush) fsmFlush;       
        stream_write_t *ptStreamWrite;
        enqueue_fn *fnEnqueue;   
    )
)



struct stream_write_t {
    bool bBusy ;
    mem_blk_t *ptFifoSend ;
    mem_blk_t *ptByteFifo ;
    mem_blk_t *ptByteFifoCpl ;
    mem_blk_fifo_t tMemBlockFifo ;
    void (*fnDmaStart)(mem_blk_t *ptFifoSend) ;
    byte_fifo_t tByteFifo ; 
    fsm(enqueue) fsmEnqueue ;
    fsm(stream_write_flush) fsmFlushAuto ;
    fsm(stream_write_flush) fsmFlushManual ;
} ;







extern
void stream_write_init(stream_write_t *ptThis,stream_write_cfg_t *ptCfg) ;
extern
bool stream_write(stream_write_t *ptThis,byte tByte) ;
extern
void dma_send_data_cpl_event_handler(stream_write_t *ptThis) ;

#endif

