#ifndef __FSM_DEQUEUE_H__
#define __FSM_DEQUEUE_H__

/*============================ INCLUDES ======================================*/

#include ".\stream_read.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/
extern 
    void record_current_data_count(stream_read_t *ptThis);
extern 
    void set_dma_idle(stream_read_t *ptThis);
extern
    void set_uart_idle(stream_read_t *ptThis);
extern
    void set_uart_busy(stream_read_t *ptThis);
extern
    void  set_target_time(stream_read_t *ptThis);
#endif