#include "sht31.h"
#include "myiic.h"
#include "FreeRTOS.h"
#include "task.h"


/*
函数功能：初始化IIC软件引脚
形参：void
返回值：void 
函数说明：
*/
void IIC_Software_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	 
	GPIO_InitTypeDef  GPIO_InitStruct={0};
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;  //通用推挽输出
	GPIO_InitStruct.GPIO_Speed =  GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//SCL和SDA默认电平
	IIC_SDA_High;  
	IIC_SCL_High;
	
}



/* 
函数功能：起始信号
形参：void
返回值：void 
函数说明：
*/
static void IIC_Start(void)
{
	//注意：在起始信号和停止信号中，先动SDA再动SCL
	IIC_SDA_High;  //有上拉电阻拉高
	IIC_SCL_High;  //拉高时钟线
	IIC_Delay(); //电平保持时间
	IIC_SDA_Low;   //拉低数据线，进入起始信号
	IIC_Delay(); //电平变化时间
}


/* 
函数功能：停止信号
形参：void
返回值：void 
函数说明：
*/
static void IIC_Stop(void)
{
	IIC_SDA_Low;  //拉低数据线
	IIC_SCL_High;  //拉高时钟线
	IIC_Delay(); //电平保持时间
	IIC_SDA_High;   //拉高数据线
	IIC_Delay(); //电平变化时间
}

/* 
函数功能：主机发送1bit应答
形参：u8 ack   主机发送应答数据
返回值：void 
函数说明：
*/
static void IIC_SendAck(u8 ack)
{
	IIC_SCL_Low;  //拉低时钟线，主机准备数据发送从机
	IIC_Delay(); //电平变化时间
	if(ack)
	{
		IIC_SDA_High;  //发送数据1，非应答
	}
	else
	{
		IIC_SDA_Low;  //发送数据0，发送应答
	}
	IIC_Delay(); //低电平保持时间
	IIC_SCL_High;    //拉高时钟线，从机准备接收主机发送应答数据
	IIC_Delay(); //电平变化时间
	
	IIC_SCL_Low;  //拉低时钟线，保证数据完整性
	IIC_Delay(); //电平变化时间
}

/* 
函数功能：主机接收1bit应答
形参：void
返回值：u8    0:接收到应答（正常响应）  1：接收非应答 
函数说明：
*/
static u8 IIC_RecAck(void)
{
	u8 ack=0;
	//SDA_IO切换为输入功能
	IIC_SDA_High;
	
	IIC_SCL_Low;  //拉低时钟线，从机发送数据到主机
	IIC_Delay(); //电平变化时间
	IIC_SCL_High;  //拉高时钟线，主机接收从机应答
	IIC_Delay(); //电平变化时间
	if(IIC_SDA_IN)
	{
		ack=1;
	}
	
	IIC_SCL_Low;  //拉低时钟线，保证数据完整性
	IIC_Delay(); //电平变化时间
	
	return ack;
}


/* 
函数功能：IIC发送8bit数据
形参：u8 data  需要发送的8bit数据
返回值：void 
函数说明：
*/
static void IIC_SendData(u8 data)
{
	for(u8 i=0;i<8;i++)
	{
		IIC_SCL_Low; //拉低时钟线，主机向从机发送数据
		IIC_Delay(); //电平变化时间
		if(data & (0x80 >> i))
		{
			IIC_SDA_High;  //由上拉电阻输出数据1
		}
		else
		{
			IIC_SDA_Low;  //拉低数据线，输出数据0
		}
		IIC_Delay(); //电平变化时间
		IIC_SCL_High;  //拉高时钟线，
	    IIC_Delay(); //电平变化时间
	}

}

/* 
函数功能：IIC接收8bit数据
形参：void
返回值：u8     
函数说明：
*/
static u8 IIC_RecData(void)
{
	u8 temp=0;
	//SDA_IO切换为输入功能
	IIC_SDA_High;
	
	for(u8 i=0;i<8;i++)
	{
		IIC_SCL_Low;  //拉低时钟线，从机发送数据到主机
		IIC_Delay(); //电平变化时间
		IIC_SCL_High;  //拉高时钟线，
	    IIC_Delay(); //电平变化时间
		
		temp<<=1;
		if(IIC_SDA_IN) //接收到数据1
		{
			temp |=1;
		}
	}
	return temp;
	
}


//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	IIC_SDA_IN;//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL_Low; 
        IIC_Delay();
		IIC_SCL_High;
        receive<<=1;
        if(IIC_SDA_IN)receive++;   
		IIC_Delay(); 
    }					 
    if (!ack)
        IIC_SendAck(1);//发送nACK
    else
        IIC_SendAck(0); //发送ACK   
    return receive;
}



float temp_val=0;
float hum_val=0;

/**
 * @brief  读取SHT31温湿度数据
 * @param void
 * @retval 0-成功, 1-失败
 */
u8 SHT31_ReadData(void) 
{

	u8 str[6]={0};
    
	IIC_Start();
	IIC_SendData(0x88);

	if(IIC_RecAck())
	{
		IIC_Stop();
		return 1;
	}

	IIC_SendData(0x2C);
	if(IIC_RecAck())
	{
		IIC_Stop();
		return 2;
	}

	IIC_SendData(0x06);
	if(IIC_RecAck())
	{
		IIC_Stop();
		return 3;
	}
	IIC_Stop();
	
	vTaskDelay(15);
	IIC_Start();
	IIC_SendData(0x89);
	if(IIC_RecAck())
	{
		IIC_Stop();
		return 4;
	}
	
	for(u8 i=0;i<5;i++)
	{
		str[i] = IIC_RecData();
		IIC_SendAck(0);
	}
	str[5] = IIC_RecData();
	IIC_SendAck(1);
	IIC_Stop();
	
	float temp = (str[0] <<8)| str[1];
	float hum  = (str[3] <<8)| str[4];
	
	temp_val = (175*temp)/(65536-1) - 45;
	hum_val = 100*hum/(65536-1);
	
	return 0;
}
