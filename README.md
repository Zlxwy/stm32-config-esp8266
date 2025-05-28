This is my first GitHub repository project. &#x1F602;&#x1F602;

# 目录
* [项目简介](#项目简介)
* [分别介绍](#分别介绍)
  * [关于esp8266连接原子云平台-stm32f103](#关于esp8266连接原子云平台-stm32f103)
  * [关于esp8266连接原子云平台-stm32f407](#关于esp8266连接原子云平台-stm32f407)

# 项目简介
- 使用stm32芯片，通过AT指令自动配置esp8266连接原子云.
  - 包含stm32f103系列的程序，以stm32f103c8t6为例
  - 包含stm32f407系列的程序，以stm32f407zgt6为例

# 分别介绍
## 关于esp8266连接原子云平台-stm32f103
### 器件
- STM32F103C8T6核心板
- ESP8266模块，必须是正点原子的ESP8266模块才能连接上原子云平台
- OLED四线I2C显示屏，SSD1306芯片的

### 简介
串口3配置ESP8266连接原子云，并能和原子云互相通信。

### 配置
- USART3: 连接ESP8266的串口
- TIM2: 辅助USART3接收ESP8266消息的定时器
- PB8: GPIO模拟I2C_SCL
- PB9: GPIO模拟I2C_SDA

### 连线
| 引脚 | 外设功能 | 连接设备 | 作用 |
| --- | --- | --- | --- |
| PB10 | USART3_TX | ESP8266_RXD | stm32向ESP8266发送数据 |
| PB11 | USART3_RX | ESP8266_TXD | stm32接收来自ESP8266的数据 |
| PB8 | GPIO_OUTPUT | OLED_SCL | OLED显示屏，显示配置状态信息 |
| PB9 | GPIO_OUTPUT | OLED_SDA | OLED显示屏，显示配置状态信息 |

### 现象
- 串口3连接ESP8266配置并通信，
- 上电后，USART3不断发送AT指令配置ESP8266连接原子云，在OLED上打印状态信息，
- 连接上原子云后
  - 在原子云发送的消息，可被ESP8266接收到，并由USART3_RX引脚获取，在OLED上显示


## 关于esp8266连接原子云平台-stm32f407
### 器件
- STM32F407ZGT6核心板
- USB转串口模块，用于调试
- ESP8266模块，必须是正点原子的ESP8266模块才能连接上原子云平台
- OLED四线I2C显示屏，SSD1306芯片的(可不连)

### 简介
- 串口3配置ESP8266连接原子云，并能和原子云互相通信。
- 串口1打印调试信息。

### 配置
- USART1: 调试串口
- USART3: 连接ESP8266的串口
- TIM2: 辅助USART3接收ESP8266消息的定时器
- PF1: GPIO连接OLED_SCL(可不连OLED)
- PF0: GPIO连接OLED_SDA(可不连OLED)

### 连线
| 引脚 | 外设功能 | 连接设备 | 作用 |
| --- | --- | --- | --- |
| PB6 | USART1_TX | 串口模块RXD | 串口助手向stm32发送数据 |
| PB7 | USART1_RX | 串口模块TXD | 串口助手接收来自stm32的数据 |
| PB10 | USART3_TX | ESP8266_RXD | stm32向ESP8266发送数据 |
| PB11 | USART3_RX | ESP8266_TXD | stm32接收来自ESP8266的数据 |
| PF1 | GPIO_OUTPUT | OLED_SCL | OLED显示屏，显示配置状态信息 |
| PF0 | GPIO_OUTPUT | OLED_SDA | OLED显示屏，显示配置状态信息 |

### 现象
- 串口1连接电脑串口助手调试，串口3连接ESP8266配置并通信，
- 上电后，USART3不断发送AT指令配置ESP8266连接原子云，在串口1上打印状态信息，
- 连接上原子云后，
  - 在原子云发送的消息，可被ESP8266接收到，并由USART3_RX引脚获取，
- 连接上原子云后，
  - 也可以通过串口1在电脑串口助手发送消息，USART1接收到消息后，在接收中断中将信息通过USART3_TX发送到ESP8266_RXD，
  - ESP8266_RXD接收到消息后，将消息上传至原子云，可在原子云查看消息。

