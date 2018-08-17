//****************************************************************//
//           DHT11数字温湿度传感器
//单片机 AT89S52 或 STC89C52RC
//功能   蓝牙发送温湿度数据 晶振 11.0592M 波特率 9600
//硬件   P2.0口为通讯口连接DHT11,DHT11的电源和地连接单片机的电源和地
//公司   大信物联网 https://daxiniot.taobao.com
//****************************************************************//

#include <reg51.h>
#include <intrins.h>

typedef unsigned char  U8;       /* defined for unsigned 8-bits integer variable           无符号8位整型变量  */
typedef unsigned int   U16;      /* defined for unsigned 16-bits integer variable           无符号16位整型变量 */
#define uchar unsigned char
#define uint unsigned int

//----------------IO口定义区--------------------//
sbit  P2_0  = P2^0 ;
sbit fan=P2^5;
//----------------定义区--------------------//
U8  U8temp;
U8  humidity_H,humidity_L,temperature_H,temperature_L,checkdata;
U8  str[5]= {"RS232"};

void Delay(U16 j)
{   U8 i;
    for(; j>0; j--)
    {
        for(i=0; i<27; i++);
    }
}
/**
*串口发送一个字节
*/
void sendOneChar(uchar a)
{
    SBUF = a;
    while(TI==0);
    TI=0;
}


/**
*延时函数：单位 ms
*/
void delayms(uint x)
{
    uint i,j;
    for(i=x; i>0; i--)
        for(j=110; j>0; j--);
}
/**
*延时函数： 延时10us
*/
void  Delay_10us(void)
{
    U8 i;
    i--;
    i--;
    i--;
    i--;
    i--;
    i--;
}

/**
*延时函数： 延时10us
*/
void delay13us()
{
	U8 i;
	i--;
	i--;
	i--;
	i--;
	i--;
	i--;
	i--;
	i--;
}

/**
*根据时序计算温湿度值
*/
U8 computeData()
{
    U8 i,U8comdata;
    for(i=0; i<8; i++)
    {
        while(P2_0==0);				//等待54us低电平
        Delay_10us();					//延时30us确认高电平是否结束
        Delay_10us();
        Delay_10us();
        U8temp=0;
        if(P2_0==1)						//如果高电平高过预定0高电平
        {											//值则数据位为 1
            U8temp=1;
        }
        while(P2_0==1);			//等待高电平结束
        U8comdata<<=1;
        U8comdata|=U8temp;
    }
    return U8comdata;
}

//--------------------------------
//-----湿度读取子程序 ------------
//--------------------------------
//----以下变量均为全局变量--------
//----温度高8位== temperature_H------
//----温度低8位== temperature_L------
//----湿度高8位== humidity_H-----
//----湿度低8位== humidity_L-----
//----校验 8位 == checkdata-----
//--------------------------------
void readData()
{
    U8  humidity_H_temp,humidity_L_temp,temperature_H_temp,temperature_L_temp,checkdata_temp;
    //主机拉低不少于18ms
    P2_0=0;
    delayms(20);
		//总线由上拉电阻拉高 主机延时13us最多20us 等待DHT11低电平响应
    P2_0=1;
		delay13us();
    //判断从机是否有低电平响应信号
    if(P2_0==0)
    {
        while(P2_0==0);				//判断DHT11发出 83us 的低电平响应信号是否结束
        while(P2_0==1);				//判断DHT11发出 87us 的高电平是否结束
        //数据接收状态
        humidity_H_temp = computeData();
        humidity_L_temp = computeData();
        temperature_H_temp = computeData();
        temperature_L_temp = computeData();
        checkdata_temp = computeData();
        P2_0 = 1;		//data拉高释放总线
        //数据校验
        if(checkdata_temp = humidity_H_temp + humidity_L_temp + temperature_H_temp + temperature_L_temp)
        {
            humidity_H = humidity_H_temp;
            humidity_L = humidity_L_temp;
            temperature_H = temperature_H_temp;
            temperature_L = temperature_L_temp;
            checkdata = checkdata_temp;
        }
    }

}

//----------------------------------------------
//main()功能描述:  STC89C52RC  11.0592MHz   串口发送温湿度数据,波特率 9600
//----------------------------------------------
void main()
{
    U8  i;
    TMOD = 0x20;          //定时器T1使用工作方式2
    TH1 = 253;        // 设置初值
    TL1 = 253;
    TR1 = 1;          // 开始计时
    SCON = 0x50;          //工作方式1，波特率9600bps，允许接收
    ES = 1;
    EA = 1;           // 打开所以中断
    TI = 0;
    RI = 0;
    Delay(1);         //延时100US（12M晶振)
    while(1)
    {
        //调用温湿度读取子程序
        readData();
        str[0]=humidity_H;
        str[1]=humidity_L;
        str[2]=temperature_H;
        str[3]=temperature_L;
        str[4]=checkdata;
        //发送到串口
        for(i=0; i<5; i++)
        {
            sendOneChar(str[i]);
        }
        //读取模块数据周期不易小于 2S
        delayms(2000);
    }

}

/**************************串口中断处理函数*****************************/
void uart() interrupt 4
{
	uchar a;
	if(RI==1)
	{
		RI=0;
		a=SBUF;
		if(a=='0')
		{
			fan=0;
		}
		if(a=='1')
		{
			fan=1;
		}
		
	}
    
}
