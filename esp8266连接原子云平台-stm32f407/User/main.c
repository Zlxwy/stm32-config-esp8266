#include "stm32f4xx.h" // Device header
#include "delay.h"
#include "sys.h"
#include "usart1cfg.h"
#include "esp8266cfg.h"
#include "OLED.h"

int main(void)
{
    delay_init(168);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //设置中断优先级分组2

    /*调试用*/
    USART1_Config(115200); // 连接串口助手，打印配置信息
    OLED_Init(); // OLED也是显示配置信息的，接不接都无所谓
    OLED_Clear();
    
    /*ESP8266*/
    ESP8266_Init(); // 初始化ESP8266模块，包含USART3和TIM2
    // USART3接收中断优先级0：0
    // TIM2定时中断优先级1：0
    delay_ms(2500);

    clear_rx_pack(); // 清空接收缓存区
    OLED_ShowString(1, 1, "Receiving..."); // 已经初始化完毕，进入待接收状态
    usart1_printf("ESP8266_Init OK! Receving...\r\n"); // 打印初始化完成信息

    delay_ms(1000);
    
    while (1)
    {
        if (get_TC_flag())
        {
            OLED_Clear(); //清空先前接收的消息，避免信息重叠显示
            OLED_ShowString_FromLineToLine(1, (char*)get_rx_pack(), 4);
            usart1_printf("Received: %s\r\n", get_rx_pack());
        }
    }
}

// USART1是连接电脑串口助手的，电脑发送消息后，会来到这个中断函数
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // 是接收中断
    {
        usart3_printf("%c", USART_ReceiveData(USART1)); // USART1接收串口助手发送的信息，通过USART3发送到原子云
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
