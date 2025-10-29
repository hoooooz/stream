
/*============================ INCLUDES ======================================*/

#include "bsp.h"

#include <RTE_Components.h>
#undef __USE_EVENT_RECORDER__
#if defined(RTE_Compiler_EventRecorder) || defined(RTE_CMSIS_View_EventRecorder)
#   define __USE_EVENT_RECORDER__  1
#endif

#if __USE_EVENT_RECORDER__
#   include <EventRecorder.h>
#   include "EventRecorderConf.h"
#endif


#include "..\..\stream\stream.h"

/*============================ MACROS ========================================*/
#undef   this
#define  this   (*ptThis) 
 
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/

stream_read_t g_tStreamRead;
stream_write_t g_tStreamWrite;
/*============================ PROTOTYPES ====================================*/

extern void uart1_DmaSendData(void);
static void uart_dma_data_get(mem_blk_t *ptThis);
static void uart_dma_data_send(mem_blk_t *ptThis);
static uint16_t get_dma_cnt(void) ;
static void time_trigger(uint32_t wTimeOut);
/*============================ LOCAL VARIABLES ===============================*/

__attribute__((aligned(32)))
static uint8_t s_chReadBuffer[1024];
__attribute__((aligned(32)))
static uint8_t s_chWriteBuffer[1024*10];


static stream_read_cfg_t s_tStreamReadCfg = {
    .pchBuffer     = s_chReadBuffer,
    .hwSize        = sizeof(s_chReadBuffer),
    .wTimeOutMs    = 2000,
    .fnDmaStartRx  = uart_dma_data_get,
    .fnDmaCntGet   = get_dma_cnt,
    //.fnTimeTrigger = time_trigger
};

static stream_write_cfg_t s_tStreamWriteCfg = {
    .pchBuffer     = s_chWriteBuffer,
    .hwSize        = sizeof(s_chWriteBuffer),
    .fnDmaSendData = uart_dma_data_send    
};





/*============================ IMPLEMENTATION ================================*/

int main(void)
{
    bsp_init();

#if __USE_EVENT_RECORDER__    
    EventRecorderInitialize(0, 1);
#endif
    uint8_t chByte = 0x40;

    stream_read_init(&g_tStreamRead,&s_tStreamReadCfg);
    stream_write_init(&g_tStreamWrite,&s_tStreamWriteCfg);
  
    while(1) {
        uint8_t chByte;
        
        if ( stream_read(&g_tStreamRead,&chByte) ) {
            printf("%c",chByte); 
        }
    }
}

int stdout_putchar(int ch)
{   
#if 1
    uint8_t chByte = 0;
    chByte = (uint8_t)(0x000000ff&ch);

    if ( stream_write(&g_tStreamWrite,chByte) ) {
        return ch;
    }
    return -1;
#endif
}

void DMA1_Channel4_IRQHandler(void)
{
#if 1
    if ( RESET != DMA_GetITStatus(DMA1_IT_TC4) ) {
        DMA_Cmd(DMA1_Channel4,DISABLE);
        DMA_ClearITPendingBit(DMA1_IT_TC4);
 
        dma_send_data_cpl_event_handler(&g_tStreamWrite);
    }
#endif
}

static uint16_t get_dma_cnt(void) 
{   
    return DMA_GetCurrDataCounter(DMA1_Channel5);
}

void DMA1_Channel5_IRQHandler(void)
{
   
    if ( RESET != DMA_GetITStatus(DMA1_IT_HT5) ) {       
        DMA_ClearITPendingBit(DMA1_IT_HT5);
        uart_dma_get_data_insert_to_dma_irq_event_handler(&g_tStreamRead);
    }
   
    if ( RESET != DMA_GetITStatus(DMA1_IT_TC5) ) {
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
}

void USART1_IRQHandler(void)
{
    

    if ( RESET != USART_GetITStatus(USART1,USART_IT_IDLE) ) {
        uint32_t temp = USART1->SR;  
        temp = USART1->DR;
        (void)temp;

        uart_idle_insert_to_uart_irq_event_handler(&g_tStreamRead);
    }
}


static void uart_dma_data_get(mem_blk_t *ptThis)
{
    if ( NULL == ptThis ) {
        return ;
    }

    DMA_Cmd(DMA1_Channel5,DISABLE);
    DMA1_Channel5->CMAR = (uint32_t)(this.chMemory);
    DMA1_Channel5->CNDTR = this.tSizeInByte;   
    DMA_Cmd(DMA1_Channel5,ENABLE);
}

static void uart_dma_data_send(mem_blk_t *ptThis)
{
    if ( NULL == ptThis ) {
        return ;
    }
    
    DMA_Cmd(DMA1_Channel4,DISABLE);
    DMA1_Channel4->CMAR  = (uint32_t)(this.chMemory);
    DMA1_Channel4->CNDTR = this.tSizeInByte;   
    DMA_Cmd(DMA1_Channel4,ENABLE);

}

static void time_trigger(uint32_t wTimeOut)
{
    bsp_StartHardTimer(1, wTimeOut, NULL);
}


void TIM5_IRQHandler(void)
{
    uint32_t timesr;
    uint16_t itstatus = 0x0, itenable = 0x0;
    TIM_TypeDef *TIMx = TIM5;

    timesr = TIMx->SR;
    if (timesr & TIM_IT_Update) {
        TIMx->SR = (uint16_t)~TIM_IT_Update;
        uart_wait_time_out_insert_to_hard_timer_irq_event_handler(&g_tStreamRead) ;
        
    }
    
    itstatus = timesr & TIM_IT_CC1;
    itenable = TIMx->DIER & TIM_IT_CC1;
    
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET)) {    
        TIMx->SR = (uint16_t)~TIM_IT_CC1;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC1;    /* disable CC1 interupt */

       //uart_wait_time_out_insert_to_hard_timer_irq_event_handler(&g_tStreamRead) ;
    }

}


