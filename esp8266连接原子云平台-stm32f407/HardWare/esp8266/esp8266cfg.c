#include "esp8266cfg.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "stdbool.h"
#include "delay.h"
#include "OLED.h" // 用于显示ESP8266的消息
#include "usart1cfg.h" // 用于在电脑串口助手显示ESP8266的消息，和OLED保持一致

uint8_t
    usart3_rx_byte,
    usart3_rx_pack[256],
    usart3_tx_pack[256],
    usart3_rx_cnt;

volatile bool
    is_timer_enabled = false, // 定时器是否已打开
    is_usart3_rx_done = false; // 串口接收是否完成

// 定时器一直在向上计数，串口每接收到一个字节时，就会在串口中断函数里清零一次定时器的计数器，导致定时器无法进入中断，
// 当接收完数据了，串口不再产生中断，定时器计数器才能计到10ms，得以进入中断，在定时中断函数内标记接收完毕，并失能定时器。
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        usart3_rx_byte = USART_ReceiveData(USART3);
        usart3_rx_pack[usart3_rx_cnt] = usart3_rx_byte;
        if(usart3_rx_cnt<255)  usart3_rx_cnt ++;//变量usart3_rx_cnt是uint8_t，最大只能255，若接收字符超过了255个，usart3_rx_cnt保持255不再增加
        
        TIM_SetCounter(TIM2, 0);
        if (!is_timer_enabled) // 如果定时器还没打开
        {
            TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清一下标志位，防止意外进入中断
            TIM_Cmd(TIM2, ENABLE);                      //使能定时器开始配合工作  
            is_timer_enabled = true;                     //标记定时器已打开，下一次就不用再进入这个if了
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        usart3_rx_pack[usart3_rx_cnt] = '\0'; //在接收数据最后一位补上结束符
        usart3_rx_cnt = 0; //usart3_rx_cnt清零，以待下一次接收
        is_usart3_rx_done = true; //标记接收完成
        
        TIM_Cmd(TIM2, DISABLE); // 关闭定时器
        TIM_SetCounter(TIM2, 0); // 清零计数器
        is_timer_enabled = false; // 标记定时器关闭
        
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

/**用串口3发送字符串（注意!!!这个函数没有自动添加\r\n）（最多能发送255个字符）**
    *@param 像printf一样的参数格式
    **/
void usart3_printf(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char*)usart3_tx_pack, fmt, ap);
    va_end(ap);
    for(int i=0; usart3_tx_pack[i]!='\0'; i++)                          //循环发送数据
    {
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);   //发送寄存器空，才能跳出while循环
        USART_SendData(USART3, usart3_tx_pack[i]);                      //发送一个字节
    }
}

/*若接收完数据，则调用这个函数就会返回1，再次调用就不会再返回1了*/
uint8_t get_TC_flag(void)
{
    if (is_usart3_rx_done)    //若接收完一次数据了，则满足if
    {
        is_usart3_rx_done = false;
        return 1;
    }
    else return 0;
}

/**
 * @brief 获取接收到的数据包
 * @retval uint8_t* 返回指向接收到的数据包的指针
 */
uint8_t* get_rx_pack(void)
{
    return usart3_rx_pack;
}

/**清空数据包，实为把第一位变成结束符'\0'**/
void clear_rx_pack(void)
{
    usart3_rx_pack[0] = '\0';
}

/**
 * @brief 检查回应中的\r\nOK\r\n或\r\nERROR\r\n
 * @param 要检查的字符串
 * @retval 1: 回应中有\r\nOK\r\n
 * @retval 0: 回应中有\r\nERROR\r\n，或者没有找到指定回应
 */
uint8_t check_res_state(const char *str)
{
    char *ptr_OK, *ptr_ERROR;
    ptr_OK = strstr(str, "\r\nOK\r\n");
    ptr_ERROR = strstr(str, "\r\nERROR\r\n");
    if (ptr_OK != NULL) return 1;
    else if (ptr_ERROR != NULL) return 0;
    else return 0;
}


























//如果需要自定义发送的指令组，只需要更改宏定义 COMMAND_NUM 和函数 command_init() 即可
#define COMMAND_NUM    5        //需要发送指令的条数
char COMMAND[COMMAND_NUM][50];  //指令组
const char //网络端账密配置
    WIFI_SSID[30]     = "Xiaomi 12S",           // 需要连接的WiFi名称
    WIFI_PASSWORD[30] = "12345676w",            // WiFi密码
    EQUI_NUMBER[30]   = "14231768013559298853", // 原子云设备编号
    EQUI_PASSWORD[30] = "12345676";             // 原子云设备密码

/**指令组初始化——配置指令组**/
void command_init(void)
{
    sprintf(COMMAND[0],"AT\r\n");                                                   // 呼叫AT
    sprintf(COMMAND[1],"ATE0\r\n");                                                 // 关闭回显
    sprintf(COMMAND[2],"AT+CWMODE=1\r\n");                                          // 设置为STA模式
    sprintf(COMMAND[3],"AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI_SSID,WIFI_PASSWORD);       // 连接WIFI
    sprintf(COMMAND[4],"AT+ATKCLDSTA=\"%s\",\"%s\"\r\n",EQUI_NUMBER,EQUI_PASSWORD); // 连接原子云平台
}

/* 初始化ESP8266，会同时初始化串口3、定时器2，
 * 在第一行显示发送的指令，第二行到第四行显示ESP8266的回应
 */
void ESP8266_Init(void)
{
    command_init();         //指令组初始化
    esp8266_uart_init();    //串口3初始化
    esp8266_timer_init();   //定时器2初始化

    delay_ms(2000); //让8266上电后稳定了再初始化
    uint8_t state;  //如果回应含有OK了，则置1，跳出循环
    
    for (uint8_t i=0; i<COMMAND_NUM; i++)
    {
        state = 0;
        usart3_printf(COMMAND[i]);    //发送对应指令

        OLED_Clear(); //清屏
        OLED_ShowString_FromPointToLine(1, 1, COMMAND[i], 1); //第一行显示发送的指令
        usart1_printf("%s\n", COMMAND[i]);

        while (state == 0)
        {
            if (get_TC_flag())
            {
                OLED_ClearLine(2, 4); //清空2~4行先前显示的内容
                OLED_ShowString_FromLineToLine(2, (char*)usart3_rx_pack, 3); //2~3行显示接收数据包内容
                usart1_printf("%s\n", (char*)usart3_rx_pack);
                
                state = check_res_state((char*)usart3_rx_pack); //检查数据包里的\r\nOK\r\n或\r\nERROR\r\n
                //若state得到1，则表明收到了\r\nOK\r\n，ESP8266响应了指令，就能跳出while循环，继续轮入for循环发送下一条指令
                OLED_ShowNum(4, 1, state, 2); //最后一行显示回应状态
                usart1_printf("state: %02d\n", state);

                delay_ms(500); //等待，能让我看清OLED上的指令以及回应
            }
        }
    }
}

void esp8266_uart_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //PB10与PB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度100MHz
    // GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10(USART3_TX)，PB11(USART3_RX)
    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);//PB10复用为USART3_TX
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);//PB11复用为USART3_RX
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能USART3时钟
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200; //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
    USART_Init(USART3, &USART_InitStructure); //初始化串口 
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //开启接收中断
    
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //已在主函数内设置
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

void esp8266_timer_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_InternalClockConfig(TIM2);//配置定时器2使用内部时钟

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1,//时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up,//向上计数模式
    TIM_TimeBaseStructure.TIM_Prescaler = 1000-1,//定时器分频
    TIM_TimeBaseStructure.TIM_Period = 840-1; //自动重装载值
    //84M / 1000 / 840 = 100Hz更新频率，10ms定时
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);//初始化定时器2
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //已在主函数内设置
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能中断通道
    NVIC_Init(&NVIC_InitStructure); //初始化中断配置

    TIM_Cmd(TIM2, ENABLE);
}
