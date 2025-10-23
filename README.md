# stream

# stream_read 和 stream_write 使用文档

这是一份关于 `stream` 项目中 `stream_read` 和 `stream_write` 模块的使用文档。这两个模块提供了基于DMA的UART数据收发抽象层,通过有限状态机(FSM)协调异步数据传输和缓冲区管理。

## stream_read - 数据接收

### 功能概述

`stream_read` 模块通过DMA从UART硬件接收数据,并提供简单的字节流API供应用层读取。

### 初始化

#### 配置结构体

使用 `stream_read_cfg_t` 配置接收参数: 

| 字段            | 类型               | 说明                           |
| --------------- | ------------------ | ------------------------------ |
| `pchBuffer`     | `uint8_t*`         | 接收缓冲区内存池指针           |
| `hwSize`        | `uint16_t`         | 缓冲区总大小                   |
| `wTimeOutMs`    | `uint32_t`         | 定时器产生中断的设置时间(毫秒) |
| `fnDmaStartRx`  | `dma_start_rx_fn*` | DMA启动回调函数                |
| `fnDmaCntGet`   | `dma_cnt_get_fn*`  | 获取DMA剩余计数回调            |
| `fnTimeTrigger` | `time_trigger_fn*` | 触发硬件定时器回调             |

通过一个硬件定时器来生产中断，来读取剩余在fifo 里面的数据，当`fnTimeTrigger`触发时，定时器开始计时， `wTimeOutMs` 毫秒后产生中断，如果中断还没响应又触发`fnTimeTrigger`，定时器重新开始计时，`wTimeOutMs`毫秒后产生中断。

#### 初始化函数

调用 `stream_read_init()` 初始化模块: 

```c
stream_read_t g_tStreamRead;
stream_read_cfg_t s_tStreamReadCfg = {
    .pchBuffer     = s_chReadBuffer,
    .hwSize        = sizeof(s_chReadBuffer),
    .wTimeOutMs    = 2000,
    .fnDmaStartRx  = uart_dma_data_get,
    .fnDmaCntGet   = get_dma_cnt,
    .fnTimeTrigger = time_trigger
};

stream_read_init(&g_tStreamRead, &s_tStreamReadCfg);
```

### 读取数据

使用 `stream_read()` 函数逐字节读取数据: 

```c
uint8_t chByte;
if (stream_read(&g_tStreamRead, &chByte)) {
    // 成功读取一个字节
    printf("%c", chByte);
}

函数返回 `true` 表示成功读取,`false` 表示当前无数据可读。
```

### 中断处理

需要在相应的中断服务程序中调用以下事件处理函数:

#### DMA半满中断

当dma接收到字节数为设置的一半时调用：

```c
void DMA1_Channel5_IRQHandler(void) {
    if (RESET != DMA_GetITStatus(DMA1_IT_TC5)) {
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        uart_dma_get_data_insert_to_dma_irq_event_handler(&g_tStreamRead);
    }
}
```

#### UART空闲中断

当UART检测到空闲线路时调用: 

```c
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) {
        uart_idle_insert_to_uart_irq_event_handler(&g_tStreamRead);
    }
}
```
#### 超时定时器中断

当超时定时器到期时调用: 

```c
void TIMx_IRQHandler(void) {
    uart_wait_time_out_insert_to_hard_timer_irq_event_handler(&g_tStreamRead);
}
```
## stream_write - 数据发送

### 功能概述

`stream_write` 模块提供通过DMA向UART发送数据的高层接口,自动管理缓冲和DMA事务协调。 

### 初始化

#### 配置结构体

使用 `stream_write_cfg_t` 配置发送参数: 

| 字段            | 类型                | 说明                 |
| --------------- | ------------------- | -------------------- |
| `pchBuffer`     | `uint8_t*`          | 发送缓冲区内存池指针 |
| `hwSize`        | `uint16_t`          | 缓冲区总大小         |
| `fnDmaSendData` | `dma_send_data_fn*` | DMA发送启动回调函数  |

#### 初始化函数

调用 `stream_write_init()` 初始化模块: 

```c
stream_write_t g_tStreamWrite;
stream_write_cfg_t s_tStreamWriteCfg = {
    .pchBuffer     = s_chWriteBuffer,
    .hwSize        = sizeof(s_chWriteBuffer),
    .fnDmaSendData = uart_dma_data_send    
};

stream_write_init(&g_tStreamWrite, &s_tStreamWriteCfg);
```

### 发送数据

使用 `stream_write()` 函数逐字节发送数据: 

```c
uint8_t chByte = 'A';
if (stream_write(&g_tStreamWrite, chByte)) {
    // 字节已成功加入发送队列
}

函数返回 `true` 表示字节已入队,`false` 表示需要稍后重试(缓冲区满)。
```

### 中断处理

#### DMA传输完成中断

当DMA发送完成时调用: 

```c
void DMA1_Channel4_IRQHandler(void) {
    if (RESET != DMA_GetITStatus(DMA1_IT_TC4)) {
        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        dma_send_data_cpl_event_handler(&g_tStreamWrite);
    }
}
```



## 完整示例

以下是一个完整的使用示例,展示了如何初始化和使用这两个模块: 

```c
__attribute__((aligned(32)))
static uint8_t s_chReadBuffer[1024];
__attribute__((aligned(32)))
static uint8_t s_chWriteBuffer[1024];


static stream_read_cfg_t s_tStreamReadCfg = {
    .pchBuffer     = s_chReadBuffer,
    .hwSize        = sizeof(s_chReadBuffer),
    .wTimeOutMs    = 2000,
    .fnDmaStartRx  = uart_dma_data_get,
    .fnDmaCntGet   = get_dma_cnt,
    .fnTimeTrigger = time_trigger
};

static stream_write_cfg_t s_tStreamWriteCfg = {
    .pchBuffer     = s_chWriteBuffer,
    .hwSize        = sizeof(s_chWriteBuffer),
    .fnDmaSendData = uart_dma_data_send    
};

int main(void) {
    bsp_init();
    
    // 初始化读写模块
    stream_read_init(&g_tStreamRead, &s_tStreamReadCfg);
    stream_write_init(&g_tStreamWrite, &s_tStreamWriteCfg);
    
    // 主循环:读取数据并回显
    while(1) {
        uint8_t chByte;
        if (stream_read(&g_tStreamRead,&chByte)) {
            stream_write(&g_tStreamWrite,chByte);
        }
    }
}
```
## Notes

这两个模块都使用有限状态机(FSM)来管理复杂的异步操作。`stream_read` 使用三个FSM(dequeue、flush、timeout)协调数据接收,  而 `stream_write` 使用两个FSM(enqueue、flush)管理数据发送。

两个模块都依赖于底层的块内存管理器(`mem_blk_fifo_t`)和字节FIFO缓冲区(`byte_fifo_t`)来实现高效的内存管理和数据缓冲。





