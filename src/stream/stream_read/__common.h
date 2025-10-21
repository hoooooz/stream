#ifndef __STREAM_READ_COMMON__
#define __STREAM_READ_COMMON__

#include ".\stream_read.h"


extern 
    void record_current_data_count(stream_read_t *ptThis);
extern 
    void set_dma_idle(stream_read_t *ptThis);
extern
    void set_uart_idle(stream_read_t *ptThis);
extern
    void set_uart_busy(stream_read_t *ptThis);

#endif
