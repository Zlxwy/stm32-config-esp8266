#include "OLED.h"
#include "OLED_font.h"
#include "delay.h"

/***********************************************************************************
********************************软件IIC基本读写函数*********************************
***********************************************************************************/
static void IIC_Delay(void)
{
    delay_us(1);
}

static void IIC_START(void)
{
    /*    ___
     *SDA    \____
     *    ____
     *SCL     \___
     */
    OLED_SDA_SET;
    OLED_SCL_SET;
    
    OLED_SDA_CLR;
    IIC_Delay();
    OLED_SCL_CLR;
    IIC_Delay();
}

static void IIC_STOP(void)
{
    /*         ___
     *SDA ____/
     *        ____
     *SCL ___/     
     */
    OLED_SDA_CLR;
    OLED_SCL_CLR;
    
    OLED_SCL_SET;
    IIC_Delay();
    OLED_SDA_SET;
    IIC_Delay();
}

static void IIC_ACK(void)
{
    OLED_SDA_CLR;
    IIC_Delay();
    OLED_SCL_SET;
    IIC_Delay();
    OLED_SCL_CLR;
}

static void IIC_NACK(void)
{
    OLED_SDA_SET;
    IIC_Delay();
    OLED_SCL_SET;
    IIC_Delay();
    OLED_SCL_CLR;
}

static void IIC_WaitACK(void)
{
    OLED_SDA_SET;
    IIC_Delay();
    OLED_SCL_SET;
    IIC_Delay();
    OLED_SCL_CLR;
}


//单纯发送一个字节，不是指定地址写
static void IIC_SendByte(u8 byte)
{
    for(u8 i=0;i<8;i++)//循环8次，主机依次发送数据的每一位
    {
        OLED_SDA=(byte>>(7-i))&0x01;//由最高位到最低位依次发送
        IIC_Delay();//等待，让SDA线能完全改变电平
        OLED_SCL_SET;//释放SCL，从机将在SCL高电平期间读取SDA
        IIC_Delay();//等待让SCL能完全松开，并且从机读取到数据
        OLED_SCL_CLR;//拉低SCL，主机开始发送下一位数据
    }
}

//单纯接收一个字节，不是指定地址读
static u8 IIC_ReceiveByte(void)
{
    u8 byte = 0x00;//定义接收的数据
    OLED_SDA_SET;//接收前，主机先确保释放SDA，避免干扰从机的数据发送
    for(u8 i=0;i<8;i++)//循环8次，主机依次接收数据的每一位
    {
        IIC_Delay();//等待让从机能完全改变电平
        OLED_SCL_SET;//释放SCL，主机机在SCL高电平期间读取SDA
        
        byte |= OLED_SDA_READ<<(7-i);//读取SDA数据，并存储到Byte变量
        
        OLED_SCL_CLR;//拉低SCL，从机在SCL低电平期间写入SDA
    }
    return byte;//返回接收到的一个字节数据
}

/**指定地址写一个字节
    *@process   产生起始信号→发送要呼叫的从机地址+写操作→发送所呼叫从机的寄存器地址→发送要写的数据→产生停止信号
    *@param     从机未移位的原7位地址
    *@param     从机的寄存器地址
    *@param     要写入的数据
**/
static void IIC_WriteByte(u8 EquiAddr, u8 RegAddr, u8 Data)
{
    EquiAddr = (EquiAddr<<1)&0xFE;//加上读写位，这里都是写操作
    
    IIC_START();//产生起始信号
    IIC_SendByte(EquiAddr);//发送要呼叫的从机地址+写操作
    IIC_WaitACK();//接收应答
    
    IIC_SendByte(RegAddr);//发送所呼叫从机的寄存器地址
    IIC_WaitACK();//接收应答
    
    IIC_SendByte(Data);//发送要写的数据
    IIC_WaitACK();//接收应答
    IIC_STOP();//产生停止信号
}

/**指定地址写多个字节**
    *@param     从机未移位的原7位地址
    *@param     从机的寄存器地址
    *@param     存放写入数据的数组
    *@param     写入数据的长度
    **/
static void IIC_ScanWrite(u8 EquiAddr,u8 RegAddr,u8 *SendArray,u8 len)
{
    EquiAddr = (EquiAddr<<1)&0xFE;//加上读写位，这里都是写操作
    
    IIC_START();//产生起始信号
    IIC_SendByte(EquiAddr);//发送要呼叫的从机地址+写操作
    IIC_WaitACK();//接收应答
    
    IIC_SendByte(RegAddr);//发送所呼叫从机的寄存器地址
    IIC_WaitACK();//接收应答
    
    while(len--)
    {
        IIC_SendByte(*(SendArray++));//发送要写的数据
        IIC_WaitACK();//接收应答
    }
    IIC_STOP();//产生停止信号
}

/**指定地址读一个字节**
    *@process   产生起始信号→发送要呼叫的从机地址+写操作→发送所呼叫从机的寄存器地址↓
    *@process   重新产生起始信号→发送要呼叫的从机地址+读操作→读取一个字节并给非应答→产生停止信号→获取数据寄存器内容并将其返回
    *@param     从机未移位的原7位地址
    *@param     从机的寄存器地址
    *@return    读取的数据
**/
static u8 IIC_ReadByte(u8 EquiAddr, u8 RegAddr)
{
    u8
        EquiAddr_write = (EquiAddr<<1)&0xFE,
        //含有读写位（写）的从机地址（把最后一位置0）
        EquiAddr_read = (EquiAddr<<1)|0x01,
        //含有读写位（读）的从机地址（把最后一位置1）
        Data;
        //接收字节存放
    
    IIC_START();//产生起始信号
    IIC_SendByte(EquiAddr_write);//发送要呼叫的从机地址+写操作
    IIC_WaitACK();//接收应答
    
    IIC_SendByte(RegAddr);//发送所呼叫从机的寄存器地址
    IIC_WaitACK();//接收应答
    
    IIC_START();//重新产生起始条件
    IIC_SendByte(EquiAddr_read);//发送要呼叫的从机地址+读操作
    IIC_WaitACK();//接收应答
    
    Data = IIC_ReceiveByte();//读取数据
    IIC_NACK();//给从机非应答
    IIC_STOP();//产生停止信号
    return Data;
}

/**指定地址读多个字节
    *@param 从机未移位的原7位地址
    *@param 从机的寄存器地址
    *@param 存放读取数据的数组
    *@param 读取数据的长度
    **/
static void IIC_ScanRead(u8 EquiAddr,u8 RegAddr,u8 *GetArray,u8 len)
{
    u8
        EquiAddr_write = (EquiAddr<<1)&0xFE,
        //含有读写位（写）的从机地址（把最后一位置0）
        EquiAddr_read = (EquiAddr<<1)|0x01;
        //含有读写位（读）的从机地址（把最后一位置1）
    
    IIC_START();//产生起始信号
    IIC_SendByte(EquiAddr_write);//发送要呼叫的从机地址+写操作
    IIC_WaitACK();//接收应答
    
    IIC_SendByte(RegAddr);//发送所呼叫从机的寄存器地址
    IIC_WaitACK();//接收应答
    
    IIC_START();//重新产生起始条件
    IIC_SendByte(EquiAddr_read);//发送要呼叫的从机地址+读操作
    IIC_WaitACK();//接收应答
    while(len--)
    {
        *(GetArray++)=IIC_ReceiveByte();//数组不断位置自增
        if(len!=0) IIC_ACK();//还没读完，给应答继续读
        else IIC_NACK();//最后一个了，给非应答不继续读了
    }
    IIC_STOP();//产生停止信号
}



/***********************************************************************************
*************************基于IIC基本读写函数的OLED驱动函数***************************
***********************************************************************************/

#define OLED_ADDR  0x3C //未左移的7位原地址0011 1100

void OLED_WriteCommand(u8 command){IIC_WriteByte(0x3C,0x00,command);}
void OLED_WriteData(u8 data){IIC_WriteByte(0x3C,0x40,data);}

void OLED_SetCursor(u8 Y, u8 X)
{
    OLED_WriteCommand(0xB0 | Y); //设置Y位置
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4)); //设置X位置高4位
    OLED_WriteCommand(0x00 | (X & 0x0F)); //设置X位置低4位
}

void OLED_Clear(void)
{  
    u8 i,j;
    for (j=0;j<8;j++)
    {
        OLED_SetCursor(j,0);
        for(i=0;i<128;i++)
            OLED_WriteData(0x00);
    }
}

/*调用OLED初始化之前，一定要先配置好I2C0外设，初始化延时函数*/
void OLED_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_GPIO_OLED_PORT_Periph,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure=
    {
        .GPIO_Pin = GPIO_OLED_SDA_PIN|GPIO_OLED_SCL_PIN,//两个连接LED负极的输出引脚
        .GPIO_Mode = GPIO_Mode_OUT,//普通输出模式
        .GPIO_OType = GPIO_OType_PP,//推挽输出
        .GPIO_Speed = GPIO_Speed_100MHz,//100MHz
        .GPIO_PuPd = GPIO_PuPd_UP//上拉
    };
    GPIO_Init(GPIO_OLED_PORT,&GPIO_InitStructure);//初始化
    OLED_SDA_SET;
    OLED_SCL_SET;
    
    delay_ms(100);
    OLED_WriteCommand(0xAE); //关闭显示
    OLED_WriteCommand(0xD5); //设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); //设置多路复用率
    OLED_WriteCommand(0x3F); 
    OLED_WriteCommand(0xD3); //设置显示偏移
    OLED_WriteCommand(0x00); 
    OLED_WriteCommand(0x40); //设置显示开始行
    OLED_WriteCommand(0xA1); //设置左右方向，0xA1正常 0xA0左右反置
    OLED_WriteCommand(0xC8); //设置上下方向，0xC8正常 0xC0上下反置
    OLED_WriteCommand(0xDA); //设置COM引脚硬件配置
    OLED_WriteCommand(0x12); 
    OLED_WriteCommand(0x81); //设置对比度控制
    OLED_WriteCommand(0xCF); 
    OLED_WriteCommand(0xD9); //设置预充电周期
    OLED_WriteCommand(0xF1); 
    OLED_WriteCommand(0xDB); //设置VCOMH取消选择级别
    OLED_WriteCommand(0x30); 
    OLED_WriteCommand(0xA4); //设置整个显示打开/关闭
    OLED_WriteCommand(0xA6); //设置正常/倒转显示
    OLED_WriteCommand(0x8D); //设置充电泵
    OLED_WriteCommand(0x14); 
    OLED_WriteCommand(0xAF); //开启显示
    OLED_Clear(); //OLED清屏
}

/**OLED显示一个字符**
    *@param  Line 行位置，范围：1~4
    *@param  Column 列位置，范围：1~16
    *@param  Char 要显示的一个字符，范围：ASCII可见字符
**/
void OLED_ShowChar(u8 Line, u8 Column, char Char)
{
    u8 i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8); //设置光标位置在上半部分
    for (i=0;i<8;i++)
        OLED_WriteData(OLED_F8x16[Char - ' '][i]); //显示上半部分内容
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8); //设置光标位置在下半部分
    for (i=0;i<8;i++)
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]); //显示下半部分内容
}

/**清除指定行列内容**
    *@param 起始行/起始列
    *@param 结束行/结束列
**/
void OLED_ClearLine(u8 line1, u8 line2)
{
    for(u8 i=line1;i<=line2;i++)
        for(u8 j=1;j<=16;j++)
            OLED_ShowChar(i,j,' ');
}
void OLED_ClearColumn(u8 column1,u8 column2)
{
    for(u8 i=column1;i<=column2;i++)
        for(u8 j=1;j<=4;j++)
            OLED_ShowChar(j,i,' ');
}

/**OLED在指定位置显示字符串，单行显示，不自动换行**
  * @param Line 起始行位置，范围：1~4
  * @param Column 起始列位置，范围：1~16
  * @param String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(u8 Line, u8 Column, char *String)
{
    u8 i;
    for (i=0;String[i]!='\0';i++)
        OLED_ShowChar(Line, Column + i, String[i]);
}

/**OLED从起始行显示字符串，到结束行终止显示，可自动换行**
    *@ATTENTION 本函数跳过字符\r和字符\n，不会显示出来
    *@param 起始行
    *@param 显示的字符
    *@param 结束行
    **/
void OLED_ShowString_FromLineToLine(u8 startline,char *str,u8 endline)
{
    for(u8 i=0,j=1;str[i]!='\0';i++)
        if (str[i]!='\r'&&str[i]!='\n')
        {
            OLED_ShowChar(startline,j++,str[i]);
            if (j>=17&&startline<endline) {startline++;j=1;}
            else if (j>=17&&startline==endline) break;
        }
}

/**OLED从指定位置显示字符串，到结束行终止显示，可自动换行**
    *@ATTENTION 本函数跳过字符\r和字符\n，不会显示出来
    *@param 起始行
    *@param 起始列
    *@param 显示字符串
    *@param 终止行
    **/
void OLED_ShowString_FromPointToLine(u8 Line,u8 Column,char *String,u8 endline)
{
    u8 i,j; //j用来保持字符串稳定增加
    for (i=0,j=0; String[j]!='\0';j++)
    {
        if (String[j]!='\r'&&String[j]!='\n')
        {OLED_ShowChar(Line,Column+i,String[j]);i++;}
        if ((Column+i)==17&&Line<endline) {Line+=1;Column=1;i=0;}
        else if ((Column+i)==17&&Line==endline) break;
    }
}

/**OLED次方函数**
    *@retval 返回值等于X的Y次方
  */
static u32 OLED_Pow(u32 X, u32 Y)
{
    u32 Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**OLED显示数字（十进制，正数)**
    *@param Line 起始行位置，范围：1~4
    *@param Column 起始列位置，范围：1~16
    *@param Number 要显示的数字，范围：0~4294967295
    *@param Length 要显示数字的长度，范围：1~10
    *@retval 无
**/
void OLED_ShowNum(u8 Line,u8 Column,u32 Number,u8 Length)
{
    for(u8 i=0;i<Length;i++)
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
}

/**OLED显示数字（十进制，带符号数）**
    *@param Line 起始行位置，范围：1~4
    *@param Column 起始列位置，范围：1~16
    *@param Number 要显示的数字，范围：-2147483648~2147483647
    *@param Length 要显示数字的长度，范围：1~10
    *@retval 无
**/
void OLED_ShowSignedNum(u8 Line,u8 Column,s32 Number,u8 Length)
{
    u8 i;
    u32 Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for(i=0;i<Length;i++)
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
}

/**OLED显示数字（十六进制，正数）**
    *@param Line 起始行位置，范围：1~4
    *@param Column 起始列位置，范围：1~16
    *@param Number 要显示的数字，范围：0~0xFFFFFFFF
    *@param Length 要显示数字的长度，范围：1~8
    *@retval 无
**/
void OLED_ShowHexNum(u8 Line,u8 Column,u32 Number,u8 Length)
{
    u8 i,SingleNumber;
    for(i=0;i<Length;i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        else
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
    }
}

/**OLED显示数字（二进制，正数）**
    *@param Line 起始行位置，范围：1~4
    *@param Column 起始列位置，范围：1~16
    *@param Number 要显示的数字，范围：0~1111 1111 1111 1111
    *@param Length 要显示数字的长度，范围：1~16
    *@retval 无
**/
void OLED_ShowBinNum(u8 Line,u8 Column,u32 Number,u8 Length)
{
    for(u8 i=0;i<Length;i++)
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
}

