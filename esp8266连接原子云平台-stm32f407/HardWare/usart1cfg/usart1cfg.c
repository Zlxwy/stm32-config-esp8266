#include "usart1cfg.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

uint8_t usart1_tx_pack[256]; // 用于发送的

void USART1_Config(uint32_t BoudRate)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIOB时钟
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // PB6与PB7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
    // GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure); // 初始化PB6(USART1_TX)，PB7(USART1_RX)
    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //PB6复用为USART1_TX
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1); //PB7复用为USART1_RX
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); //使能USART1时钟
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = BoudRate; //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
    USART_Init(USART1,&USART_InitStructure); //初始化串口1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //开启接收中断
    
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //已在主函数内设置
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;//响应优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART1, ENABLE);
}

void usart1_printf(char* fmt, ...)
{
    va_list ap;//定义一个va_list类型的变量ap，用于迭代访问函数的可变参数列表
    va_start(ap, fmt);//初始化ap，使其指向函数参数列表中的第一个参数（实为紧跟在fmt后面的参数）
    vsprintf((char*)usart1_tx_pack, fmt, ap);//将fmt字符串和后续的参数格式化成字符串，存入String数组中。操作与sprintf类似，但vsprintf可以处理可变数量的参数
    va_end(ap);//清理ap，释放与可变参数列表相关的资源。这是使用va_list时必须要做的，以确保内存管理的正确性
    uint16_t len = strlen((char*)usart1_tx_pack);//此次发送数据的长度
    for(uint16_t i=0; i<len; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//判断Transmit Empty标志位，直到该标志位置起
        USART_SendData(USART1, usart1_tx_pack[i]);
    }
}

