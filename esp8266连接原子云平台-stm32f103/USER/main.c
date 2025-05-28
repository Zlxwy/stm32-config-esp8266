#include "stm32f10x.h"
#include "string.h"
#include "delay.h"
#include "OLED.h"
#include "esp8266cfg.h"

int main(void)
{
    delay_init();
    OLED_Init();
    OLED_Clear();
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // USART3接收中断优先级0：0
    // TIM2定时中断优先级1：0
    
    ESP8266_Init();    //ESP8266（根据指令组内容）初始化
    delay_ms(2500);
    
    clear_rx_pack();
    OLED_ShowString(1, 1, "Receiving...");//已经初始化完毕，进入待接收状态
    delay_ms(1000);
    
    while(1)
    {
        if (get_TC_flag())
        {
            OLED_Clear(); //清空先前接收的消息，避免信息重叠显示
            OLED_ShowString_FromLineToLine(1, (char*)get_rx_pack(), 4);
        }
    }
}
